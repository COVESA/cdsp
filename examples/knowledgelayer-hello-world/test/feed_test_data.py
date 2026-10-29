#!/usr/bin/env python3
"""Proof that the S2DM-generated IL/KL config works end to end.

Feeds all 7 in-scope CDSP signals into the Information Layer over its
websocket API (mimicking rl-bridge.py's multi-signal "set" message format),
confirming every generated data point is reachable, then triggers KL's
driving-style reasoning rules and reads the inference result back out of IL
to prove the full loop -- S2DM-generated vspec (IL) and SHACL (KL), and the
reasoning rules -- actually works, not just that files were produced.

Sending all signals in a batch as ONE combined "set" message (rather than one
per signal) makes them land in the same IoTDB write and the same poll cycle,
avoiding both the 0.2s poll-race (a signal landing right on a poll boundary
can be silently missed) and second-level timestamp mismatches between signals
that some rules require to align exactly (e.g. FixPoint's lat/long match).

The angle swing (diff=220) clears both reasoners' thresholds: RDF4J's rules
require > 180, RDFox's require > 210.

Requires: iotdb-service, information-layer, knowledge-layer(-rdf4j or -rdfox),
and the reasoner service already running (docker compose up ...).

Usage:
    uv run --with websockets python feed_test_data.py
"""
import asyncio
import json
import time

import websockets

IL_URL = "ws://localhost:8080"
VIN = "VINABCD1234567890"

# All 7 signals cdsp-custom.graphql generates config for. Sent together to prove
# every generated data point is reachable, not just the 4 the driving-style rules
# actually consume.
BASELINE_SIGNALS = {
    "Chassis.SteeringWheel.Angle": 100,
    "Speed": 60.0,
    "CurrentLocation.Latitude": 52.5,
    "CurrentLocation.Longitude": 13.4,
    "Powertrain.TractionBattery.NominalVoltage": 400,
    "Powertrain.TractionBattery.StateOfCharge.CurrentEnergy": 55.0,
    "Powertrain.Transmission.CurrentGear": 3,
}


def to_nested(flat: dict) -> dict:
    data: dict = {}
    for dotted_path, value in flat.items():
        parts = dotted_path.split(".")
        level = data
        for part in parts[:-1]:
            level = level.setdefault(part, {})
        level[parts[-1]] = value
    return data


async def send_set(ws, msg_id, flat_readings):
    await ws.send(json.dumps({
        "jsonrpc": "2.0",
        "method": "set",
        "id": str(msg_id),
        "params": {
            "instance": VIN,
            "schema": "Vehicle",
            "data": to_nested(flat_readings),
        },
    }))
    response = json.loads(await ws.recv())
    if "error" in response:
        raise RuntimeError(f"IL rejected set {flat_readings}: {response}")
    print(f"set {list(flat_readings)} -> ok")
    return msg_id + 1


async def get(ws, msg_id, path):
    await ws.send(json.dumps({
        "jsonrpc": "2.0",
        "method": "get",
        "id": str(msg_id),
        "params": {
            "instance": VIN,
            "schema": "Vehicle",
            "path": path,
            "format": "flat",
            "root": "relative",
        },
    }))
    response = json.loads(await ws.recv())
    return msg_id + 1, response


async def main():
    async with websockets.connect(IL_URL) as ws:
        msg_id = 1
        test_started_at = time.time()

        print("=== confirming every generated data point is reachable ===")
        msg_id = await send_set(ws, msg_id, BASELINE_SIGNALS)

        print("--- waiting 2s (within RDF4J's 3s window, distinct second) ---")
        await asyncio.sleep(2)

        print("=== large angle swing (diff=220 > 210 rdfox / 180 rdf4j), moved location, still high speed ===")
        angle_swing = dict(BASELINE_SIGNALS)
        angle_swing["Chassis.SteeringWheel.Angle"] = -120
        angle_swing["CurrentLocation.Latitude"] = 52.6
        angle_swing["CurrentLocation.Longitude"] = 13.5
        msg_id = await send_set(ws, msg_id, angle_swing)

        print("--- polling for KL to reason and report a fresh result back to IL ---")
        # This result can be old. IoTDB saves its data to disk, so a `get` right after
        # sending data might return a value from an earlier run, not this one. And
        # since every run sends the same test values, checking the values won't catch
        # that. So instead, we check the timestamp: only accept a result generated
        # after this run started.
        inference = None
        for attempt in range(15):
            await asyncio.sleep(1)
            msg_id, response = await get(ws, msg_id, "AI.Reasoner.InferenceResults")
            result = response.get("result", {})
            result_str = result.get("data", {}).get("")
            generated_at = (
                result.get("metadata", {}).get("", {}).get("timestamps", {}).get("generated", {}).get("seconds")
            )
            if not result_str or generated_at is None or generated_at < test_started_at:
                print(f"  attempt {attempt + 1}: no fresh result yet, retrying...")
                continue
            inference = json.loads(result_str)
            break

        if inference is None:
            raise AssertionError(
                "Never saw a fresh inference result from KL within the timeout -- integration is broken."
            )

        def close_enough(want, got):
            return isinstance(got, (int, float)) and abs(got - want) < 0.01

        expected = {
            "DrivingStyle.AvgAngleChange": 220.0,
            "DrivingStyle.Start.Latitude": 52.5,
            "DrivingStyle.Start.Longitude": 13.4,
            "DrivingStyle.End.Latitude": 52.6,
            "DrivingStyle.End.Longitude": 13.5,
        }
        mismatches = {
            key: (want, inference.get(key))
            for key, want in expected.items()
            if not close_enough(want, inference.get(key))
        }
        if mismatches:
            raise AssertionError(f"Fresh result had unexpected values (want, got): {mismatches}. Full result: {inference}")

        print(f"\nPASS: KL reasoned over the S2DM-generated IL/KL config and reported: {inference}")


if __name__ == "__main__":
    asyncio.run(main())
