#!/usr/bin/env python3

import argparse
import time
import asyncio
import json
import sys
from typing import Optional
from remotivelabs.broker.sync import (
    Client,
    SignalsInFrame,
    BrokerException,
    SignalIdentifier,
)
from iotdb.Session import Session
import websockets
from queue import Queue
from threading import Thread

# IoTDB connection details
ip = "127.0.0.1"
port_ = "6667"
username_ = "root"
password_ = "root"
session = None
device_id_ = "root.test2.dev1"

# WebSocket URL with the default value
websocket_url = "ws://localhost:8080"

# Global variables
websocket_queue = Queue()
websocket_thread = None
websocket = None


def setup_iotdb():
    global session
    session = Session(ip, port_, username_, password_, fetch_size=1024, zone_id="GMT+00:00")
    session.open(False)
    print("Connected to IoTDB.")


def close_iotdb():
    if session:
        session.close()
        print("Closed IoTDB connection.")


import json  # Ensure JSON parsing is available

async def websocket_handler(element_id: str):
    global websocket
    try:
        websocket = await websockets.connect(websocket_url)
        print("Connected to WebSocket.")

        message_count = 0  # Counter for sent messages

        while True:
            # Wait for the next message in the queue
            data = await asyncio.get_event_loop().run_in_executor(None, websocket_queue.get)
            if data is None:  # Use `None` to signal shutdown
                break

            # Prepare WebSocket data format
            websocket_data = {
                "type": "write",
                "tree": "VSS",
                "id": element_id,
                "uuid": "rl-bridge",
                "node": {
                    "name": data["signal_name"],
                    "value": data["signal_value"]
                }
            }

            # Send the message
            await websocket.send(json.dumps(websocket_data))
            print(f"Sent data to WebSocket: {websocket_data}")

            # Wait for the response from the server before continuing
            response = await websocket.recv()

            # Check the response type
            try:
                response_data = json.loads(response)  # Parse the response as JSON
                if response_data.get("type") != "update":
                    print(f"Unexpected response from WebSocket server: {response}")
                    exit(1)  # Exit immediately without cleanup
                else:
                    print(f"Received valid update response: {response}")
            except json.JSONDecodeError:
                print(f"Failed to parse response from WebSocket server: {response}")
                exit(1)  # Exit immediately without cleanup

            # Increment message count and log queue size every 100 messages
            message_count += 1
            if message_count % 100 == 0:
                queue_size = websocket_queue.qsize()
                print(f"Queue size after {message_count} messages: {queue_size}")

    finally:
        if websocket:
            await websocket.close()
            print("Closed WebSocket connection.")

def start_websocket_thread(element_id: str):
    global websocket_thread
    websocket_thread = Thread(target=run_websocket_loop, args=(element_id,), daemon=True)
    websocket_thread.start()


def run_websocket_loop(element_id: str):
    asyncio.run(websocket_handler(element_id))


def stop_websocket_thread():
    websocket_queue.put(None)  # Signal the WebSocket handler to exit
    if websocket_thread:
        websocket_thread.join()


def on_signals(signals_in_frame: SignalsInFrame, output_mode: str):
    for signal in signals_in_frame:
        # Convert RemotiveLabs timestamp in microseconds to milliseconds
        ts_ = int(signal.timestamp_us() / 1000)

        # Prepare data for writing
        vss_name_ = signal.name()
        if (
            vss_name_ == 'Vehicle.Chassis.Accelerator.PedalPosition' or 
            vss_name_ == 'Vehicle.Powertrain.Transmission.CurrentGear' or 
            vss_name_ == 'Vehicle.Powertrain.TractionBattery.NominalVoltage' or 
            vss_name_ == 'Vehicle.Chassis.SteeringWheel.Angle'
        ):
            vss_value_ = [str(int(signal.value()))]
        else:
            vss_value_ = [str(signal.value())]

        # Log the received data
        print(f"Received from RemotiveLabs: name={vss_name_}, value={vss_value_[0]}, timestamp={ts_}")
        
        # IoTDB writing mode
        if output_mode == "iotdb":
            iotdb_vss_name_ = ['`{}`'.format(vss_name_)]
            session.insert_str_record(device_id_, ts_, iotdb_vss_name_, vss_value_)
            print(f"Written to IoTDB: {signal.to_json()}")

        # WebSocket sending mode
        elif output_mode == "websocket":
            data = {
                "timestamp": ts_,
                "signal_name": vss_name_,
                "signal_value": vss_value_[0]
            }
            websocket_queue.put(data)  # Place data in the queue for WebSocket


stopped = False  # Global flag to signal shutdown

def run_subscribe_sample(url: str, signals: list[str], secret: Optional[str], output_mode: str, element_id: Optional[str]):
    global stopped

    client = Client(client_id="Sample client")
    client.connect(url=url, api_key=secret)

    if output_mode == "websocket" and not element_id:
        raise ValueError("--id parameter is required when output_mode is 'websocket'.")

    if output_mode == "iotdb":
        setup_iotdb()
    elif output_mode == "websocket":
        start_websocket_thread(element_id)

    client.on_signals = lambda signals_in_frame: on_signals(signals_in_frame, output_mode)

    try:
        def to_signal_id(signal: str):
            s = signal.split(":")
            if len(s) != 2:
                print("--signals must be in format namespace:signal_name")
                exit(1)
            return SignalIdentifier(s[1], s[0])

        subscription = client.subscribe(
            signals_to_subscribe_to=list(map(to_signal_id, signals)),
            changed_values_only=False,
        )
    except BrokerException as e:
        print(e)
        stop_websocket_thread()
        close_iotdb()
        sys.exit(1)  # Ensure exit after cleanup
    except Exception as e:
        print(e)
        stop_websocket_thread()
        close_iotdb()
        sys.exit(1)  # Ensure exit after cleanup

    try:
        print("Broker connection and subscription setup completed, waiting for signals...")
        while not stopped:  # Exit loop when stopped is set
            time.sleep(1)
    except KeyboardInterrupt:
        print("Keyboard interrupt received, closing")
        stop_websocket_thread()
        close_iotdb()
        sys.exit(0)


def main():
    parser = argparse.ArgumentParser(description="Provide address to RemotiveBroker")

    parser.add_argument(
        "-u",
        "--url",
        help="URL of the RemotiveBroker",
        type=str,
        required=False,
        default="http://127.0.0.1:50051",
    )

    parser.add_argument(
        "-x",
        "--x_api_key",
        help="API key is required when accessing brokers running in the cloud",
        type=str,
        required=False,
        default=None,
    )

    parser.add_argument(
        "-t",
        "--access_token",
        help="Personal or service-account access token",
        type=str,
        required=False,
        default=None,
    )

    parser.add_argument(
        "-s", "--signals", help="Signal to subscribe to", required=True, nargs="*"
    )

    parser.add_argument(
        "-o", "--output_mode",
        help="Output mode: 'iotdb' to write to IoTDB, 'websocket' to send data to WebSocket",
        choices=["iotdb", "websocket"],
        required=True
    )

    parser.add_argument(
        "-i", "--id",
        help="Element ID to include in WebSocket data (required for WebSocket mode)",
        type=str,
        required=False
    )

    try:
        args = parser.parse_args()
    except Exception as e:
        return print("Error specifying signals to use:", e)

    if len(args.signals) == 0:
        print("You must subscribe to at least one signal with --signals namespace:somesignal")
        exit(1)

    secret = args.x_api_key if args.x_api_key is not None else args.access_token
    run_subscribe_sample(args.url, args.signals, secret, args.output_mode, args.id)


if __name__ == "__main__":
    main()
