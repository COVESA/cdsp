#!/usr/bin/env python3

# Simple bridge between RemotiveLabs cloud virtual signal platform and the
# Apache IoTDB data store used in the Playground. The bridge subscribes to
# the specified signals in the cloud and writes received frames into the
# IoTDB store.
#
# Copyright (C) 2024 Renesas Electronics Corporation
#           (C) 2024 RemotiveLabs
#
# Attribution: The bridge is largely based on the 'python subscribe' sample
# from the RemotiveLabs sample repository [1].
#
# [1] https://github.com/remotivelabs/remotivelabs-samples/tree/1939efca4ce5e10a7001595bb4cbe81de4a9a74e/python/subscribe

from __future__ import annotations

import argparse
import time
from typing import Optional
from remotivelabs.broker.sync import (
    Client,
    SignalsInFrame,
    BrokerException,
    SignalIdentifier,
)
from iotdb.Session import Session


# Simple Apache IoTDB session configuration
ip = "127.0.0.1"
port_ = "6667"
username_ = "root"
password_ = "root"
session = Session(ip, port_, username_, password_, fetch_size=1024, zone_id="GMT+00:00")
device_id_ = "root.test2.dev1"

def iotdb_open_session():
    session.open(False)

def iotdb_close_session():
    session.close()

def run_subscribe_sample(url: str, signals: list[str], secret: Optional[str] = None):
    client = Client(client_id="Sample client")
    client.connect(url=url, api_key=secret)

    iotdb_open_session()

    def on_signals(signals_in_frame: SignalsInFrame):
        for signal in signals_in_frame:
            # Convert RemotiveLabs timestamp in microseconds to milliseconds
            ts_ = int(signal.timestamp_us()/1000)

            # Support a wide range of possible signals without needing knowledge
            # of the DB schema by converting data to string and asking IoTDB
            # to infer the type based on the timeseries schema.

            vss_name_ = signal.name()
            if (
                vss_name_ == 'Vehicle.Chassis.Accelerator.PedalPosition' or 
                vss_name_ == 'Vehicle.Powertrain.Transmission.CurrentGear' or 
                vss_name_ == 'Vehicle.Powertrain.TractionBattery.NominalVoltage' or 
                vss_name_ == 'Vehicle.Chassis.SteeringWheel.Angle'
            ):
                # RemotiveLabs sends higher precision data types. Convert them to
                # the int type expected for these VSS leaf nodes.
                vss_value_ = [str(int(signal.value()))]
            else:
                vss_value_ = [str(signal.value())]

            # Quote the VSS leaf node name
            iotdb_vss_name_ = ['`{}`'.format(vss_name_)]
            # IoTDB does data type inference for basic types based on the timeseries schema
            session.insert_str_record(device_id_, ts_, iotdb_vss_name_, vss_value_)
            print(signal.to_json())

    client.on_signals = on_signals

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
        iotdb_close_session()
        exit(1)
    except Exception as e:
        print(e)
        iotdb_close_session()
        exit(1)

    try:
        print(
            "Broker connection and subscription setup completed, waiting for signals..."
        )
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        subscription.cancel()
        print("Keyboard interrupt received, closing")
        iotdb_close_session()


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

    try:
        args = parser.parse_args()
    except Exception as e:
        return print("Error specifying signals to use:", e)

    if len(args.signals) == 0:
        print("You must subscribe to at least one signal with --signals namespace:somesignal")
        exit(1)

    secret = args.x_api_key if args.x_api_key is not None else args.access_token
    run_subscribe_sample(args.url, args.signals, secret)


if __name__ == "__main__":
    main()
