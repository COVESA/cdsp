#!/usr/bin/env python3

import argparse
import time
import asyncio
import json
import sys
from typing import Optional
from threading import Lock  # Lock for synchronous contexts
from asyncio import Lock as AsyncLock  # Lock for asynchronous contexts
from remotivelabs.broker.sync import (
    Client,
    SignalsInFrame,
    SignalIdentifier,
)
from iotdb.Session import Session
import websockets
from datetime import datetime

# IoTDB connection details
ip = "127.0.0.1"
port_ = "6667"
username_ = "root"
password_ = "root"
session = None
device_id_ = "root.test2.dev1"

# Information Layer URL default value, can be changed via args
information_layer_url = "ws://localhost:8080"

# Global variables
signal_map = {}
signal_map_lock = Lock()  # Lock for synchronous access
signal_map_async_lock = AsyncLock()  # Lock for asynchronous access
websocket = None
client = None
subscription = None
stopped = False  # Flag to indicate program termination


def setup_iotdb():
    global session
    session = Session(ip, port_, username_, password_, fetch_size=1024, zone_id="GMT+00:00")
    session.open(False)
    print("Connected to IoTDB.")


def close_iotdb():
    global session
    if session:
        session.close()
        print("Closed IoTDB connection.")
        session = None


def process_signals(signals_in_frame: SignalsInFrame, output_mode: str):
    """
    Processes signals and either writes to IoTDB or updates the signal_map.
    """
    global stopped
    for signal in signals_in_frame:
        if stopped:
            break

        name = signal.name()
        value_timestamp = (signal.value(), signal.timestamp_us())

        timestamp = int(signal.timestamp_us() / 1000)  # Convert microseconds to milliseconds

        if output_mode == "iotdb":
            try:
                # Write directly to IoTDB
                iotdb_name = ['`{}`'.format(name)]
                iotdb_value = [str(value_timestamp[0])]
                session.insert_str_record(device_id_, timestamp, iotdb_name, iotdb_value)
                print(f"Written to IoTDB: name={name}, value={value_timestamp[0]}, timestamp={timestamp}")
            except Exception as e:
                print(f"Error writing to IoTDB: {e}")
                if stopped:
                    break
        elif output_mode == "information-layer":
            with signal_map_lock:  # Use threading.Lock here
                signal_map[name] = value_timestamp
                print(f"Written to signal map: name={name}, value={value_timestamp[0]}, timestamp={timestamp}")

def timestamp_to_metadata_format(timestamp_micro, path=""):
    seconds = timestamp_micro // 1_000_000
    nanos = (timestamp_micro % 1_000_000) * 1_000
    return {
        path: {
            "timestamps": {
                "generated": {
                    "seconds": seconds,
                    "nanos": nanos
                }
            }
        }
    }

async def websocket_handler(element_id: str):
    """
    Sends aggregated data from the signal_map to the information-layer websocket.
    """
    global websocket
    try:
        websocket = await websockets.connect(information_layer_url)
        print("Connected to information-layer websocket.")

        while not stopped:
            async with signal_map_async_lock:  # Use asyncio.Lock here
                if not signal_map:
                    await asyncio.sleep(0.1)
                    continue

                # Prepare WebSocket data format
                websocket_data = {
                    "type": "set",
                    "instance": element_id,
                }

                if len(signal_map) == 1:
                    signal_name, signal_value_timestamp = next(iter(signal_map.items()))
                    schema, _, path = signal_name.partition('.')
                    websocket_data["schema"] = schema
                    websocket_data["path"] = path
                    websocket_data["data"] = signal_value_timestamp[0]
                    websocket_data["metadata"] = timestamp_to_metadata_format(signal_value_timestamp[1])
                else:
                    # take any signal to extract the root path, that should be identical for all cases
                    common_prefix, _ = next(iter(signal_map)).split('.', 1)
                    websocket_data["schema"] = common_prefix.rstrip('.')
                    websocket_data["data"] = {}
                    metadata_accumulation = {}

                    for name, value_timestamp in signal_map.items():
                        parts = name.split('.')
                        current_level = websocket_data["data"]
                        # Traverse through the parts except the first and last one, creating nested dictionaries as needed.
                        # First part can be removed, because it is used in the websocket_data["path"] field.
                        # Last part is stored not as dictionary, but as a value.
                        for part in parts[1:-1]:
                            current_level = current_level.setdefault(part, {})
                        # Set the value for the last part in the hierarchy
                        current_level[parts[-1]] = value_timestamp[0]
                        # Add timestamp to metadata
                        data_point_path = name.lstrip(common_prefix + ".")
                        metadata_accumulation.update(timestamp_to_metadata_format(value_timestamp[1], data_point_path))

                    # add accumulated metadata
                    websocket_data["metadata"] = metadata_accumulation
                signal_map.clear()

            try:
                await websocket.send(json.dumps(websocket_data))
                timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
                print(f"{timestamp} Sent data to information-layer: {websocket_data}")

                response = await websocket.recv()
                response_data = json.loads(response)
                if response_data.get("code") != 200:
                    safe_exit(f"Unexpected response from information-layer: {response}")
                timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
                print(f"{timestamp} Received response from information-layer: {response}")

            except Exception as e:
                if not stopped:
                    safe_exit(f"Error communicating with information-layer: {e}")

    finally:
        if websocket:
            await websocket.close()
            print("Closed connection to information-layer websocket.")


def run_subscribe_sample(url: str, signals: list[str], secret: Optional[str], output_mode: str, element_id: Optional[str]):
    """
    Configures the RemotiveLabs signal subscription and starts processing signals.
    """
    global client, subscription, stopped
    subscription = None
    client = Client(client_id="Sample client")
    client.on_signals = lambda signals_in_frame: process_signals(signals_in_frame, output_mode)


    if output_mode == "iotdb":
        setup_iotdb()

    try:
        def to_signal_id(signal: str):
            s = signal.split(":")
            if len(s) != 2:
                print("--signals must be in format namespace:signal_name")
                sys.exit(1)
            return SignalIdentifier(s[1], s[0])

        while True:
            try:
                client.connect(url=url, api_key=secret)
                subscription = client.subscribe(
                    signals_to_subscribe_to=list(map(to_signal_id, signals)),
                    changed_values_only=False,
                )
                break
                
            except Exception as e:
                print(f"Subscription to Remotive Labs broker failed: Remotive Labs recording not prepared or {e}. Retrying in 2 seconds...")
                time.sleep(2)
            
        print("Remotive Labs broker connection and subscription setup completed, waiting for signals...")

        if output_mode == "information-layer":
            asyncio.run(websocket_handler(element_id))
        else:
            while True:
                time.sleep(1)  # Keep the main thread alive for IoTDB mode
    except KeyboardInterrupt:
        print("Keyboard interrupt received, closing.")
        stopped = True
        if subscription:
            subscription.cancel()
            print("Cancelled subscription to RemotiveLabs broker.")
        close_iotdb()
    except Exception as e:
        print(f"Unexpected exception: {e}")
        safe_exit(f"Error during subscription: {e}")


def safe_exit(reason: str):
    """
    Safely exits the program by closing all resources and logging the reason.
    """
    global subscription, stopped
    print(f"Exiting: {reason}")

    stopped = True  # Signal threads to stop

    if subscription is not None:
        try:
            subscription.cancel()
            print("Cancelled subscription to RemotiveLabs broker.")
        except Exception as e:
            print(f"Error cancelling subscription: {e}")

    close_iotdb()
    sys.exit(0)

def non_empty_string(value):
    if not value.strip():
        raise argparse.ArgumentTypeError("argument cannot be an empty string.")
    return value

def main():
    parser = argparse.ArgumentParser(description="Provide address to RemotiveBroker")

    parser.add_argument(
        "-u",
        "--url",
        help="URL of the RemotiveBroker",
        type=non_empty_string,
        required=False
    )

    parser.add_argument(
        "-x",
        "--x_api_key",
        help="API key is required when accessing brokers running in the cloud",
        type=non_empty_string,
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
        "-s",
        "--signals",
        help="Signal to subscribe to",
        required=True,
        nargs="*",
    )

    parser.add_argument(
        "-o",
        "--output_mode",
        help="Output sent to iotdb or information-layer",
        choices=["iotdb", "information-layer"],
        required=True,
    )

    parser.add_argument(
        "-i",
        "--id",
        help="ID to which signals are related to (only information-layer mode)",
        type=str,
        required=False,
    )

    parser.add_argument(
        "-ilu",
        "--information_layer_url",
        help="URL of the information layer. If not provided ws://localhost:8080 will be used.",
        type=str,
        required=False,
    )

    try:
        args = parser.parse_args()
    except Exception as e:
        return print("Error specifying signals to use:", e)

    if len(args.signals) == 0:
        print("You must subscribe to at least one signal with --signals namespace:somesignal")
        exit(1)

    if args.output_mode == "information-layer" and not args.id:
        print("Error: In information-layer mode an ID has to be defined.")
        exit(1)

    if args.information_layer_url is not None:
        global information_layer_url
        information_layer_url = args.information_layer_url

    secret = args.x_api_key if args.x_api_key is not None else args.access_token

    run_subscribe_sample(args.url, args.signals, secret, args.output_mode, args.id)


if __name__ == "__main__":
    main()
