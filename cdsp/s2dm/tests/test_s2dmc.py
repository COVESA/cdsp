#!/usr/bin/env python3
import subprocess
from pathlib import Path

import pytest
from rdflib import Graph, Namespace
from rdflib.namespace import SH, XSD

PACKAGE_DIR = Path(__file__).parent.parent
SCHEMA = PACKAGE_DIR / "cdsp-custom.graphql"
NAMING_CONFIG = PACKAGE_DIR / "naming.yaml"
CAR = Namespace("http://example.ontology.com/car#")


@pytest.fixture(scope="module")
def generated(tmp_path_factory):
    out_dir = tmp_path_factory.mktemp("s2dmc_out")
    il_out = out_dir / "vss_data_points.yaml"
    shacl_out = out_dir / "vehicle_shacl.ttl"

    subprocess.run(
        [
            "s2dmc",
            "--schema", str(SCHEMA),
            "--naming-config", str(NAMING_CONFIG),
            "--il-out", str(il_out),
            "--kl-out", str(shacl_out),
        ],
        check=True,
    )
    return {"il_text": il_out.read_text(), "shacl_path": shacl_out}


def test_vspec_acronyms_are_correctly_cased(generated):
    il_text = generated["il_text"]

    assert "Vehicle.AI:" in il_text
    assert "Vehicle.AI.Reasoner.InferenceResults:" in il_text
    assert "Vehicle.PWFStatus:" in il_text
    assert "Vehicle.VehicleIdentification.VIN:" in il_text

    assert "Vehicle.Ai" not in il_text
    assert "Vehicle.PwfStatus:" not in il_text
    assert "Vehicle.VehicleIdentification.Vin:" not in il_text


def test_shacl_acronyms_are_correctly_cased(generated):
    graph = Graph()
    graph.parse(generated["shacl_path"], format="turtle")
    names = {str(o) for _, _, o in graph.triples((None, SH.name, None))}

    assert {"AI", "VIN", "PWFStatus"} <= names
    assert not names & {"Ai", "Vin", "PwfStatus"}


def test_shacl_integer_fields_use_precise_xsd_types(generated):
    graph = Graph()
    graph.parse(generated["shacl_path"], format="turtle")

    expected_datatypes = {
        CAR.Angle: XSD.short,
        CAR.NominalVoltage: XSD.unsignedShort,
        CAR.CurrentGear: XSD.byte,
        CAR.WindowPosition: XSD.unsignedByte,
        CAR.LightIntensity: XSD.unsignedByte,
    }
    for path, expected_datatype in expected_datatypes.items():
        shape = next(graph.subjects(SH.path, path))
        assert graph.value(shape, SH.datatype) == expected_datatype
