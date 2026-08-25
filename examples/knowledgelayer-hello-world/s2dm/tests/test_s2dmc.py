#!/usr/bin/env python3
import tempfile
from pathlib import Path

from rdflib import Graph, Namespace
from rdflib.namespace import SH, XSD

from s2dmc.main import fix_vspec_acronyms, fix_shacl_acronyms, fix_shacl_int_datatypes

SAMPLE_VSPEC = """\
Vehicle.Ai.Reasoner.InferenceResults:
  datatype: string
  description: Inference results.

Vehicle.VehicleIdentification.Vin:
  datatype: string

Vehicle.PwfStatus:
  datatype: string
"""

SAMPLE_SHACL = """\
@prefix car: <http://example.ontology.com/car#> .
@prefix sh: <http://www.w3.org/ns/shacl#> .
@prefix val: <http://www.w3.org/2001/XMLSchema#> .

val:Vehicle a sh:NodeShape ;
    sh:property [ sh:name "Ai" ; sh:path car:Ai ],
        [ sh:name "Vin" ; sh:path car:vin ],
        [ sh:name "PwfStatus" ; sh:path car:PwfStatus ] .
"""

def test_fix_vspec_acronyms():
    with tempfile.TemporaryDirectory() as tmp:
        raw = Path(tmp) / "raw.yaml"
        out = Path(tmp) / "out.yaml"
        raw.write_text(SAMPLE_VSPEC)

        fix_vspec_acronyms(raw, out)
        result = out.read_text()

        assert "Vehicle.AI.Reasoner.InferenceResults:" in result
        assert "Vehicle.VehicleIdentification.VIN:" in result
        assert "Vehicle.PWFStatus:" in result
        assert "Vehicle.Ai.Reasoner.InferenceResults:" not in result
        assert "Vehicle.VehicleIdentification.Vin:" not in result
        assert "Vehicle.PwfStatus:" not in result

def test_fix_shacl_acronyms():
    with tempfile.TemporaryDirectory() as tmp:
        raw = Path(tmp) / "raw.ttl"
        raw.write_text(SAMPLE_SHACL)

        graph = fix_shacl_acronyms(raw)
        serialized = graph.serialize(format="turtle")

        assert 'sh:name "AI"' in serialized
        assert 'sh:name "VIN"' in serialized
        assert 'sh:name "PWFStatus"' in serialized
        assert 'sh:name "Ai"' not in serialized
        assert 'sh:name "Vin"' not in serialized
        assert 'sh:name "PwfStatus"' not in serialized

SAMPLE_SHACL_INT = """\
@prefix car: <http://example.ontology.com/car#> .
@prefix sh: <http://www.w3.org/ns/shacl#> .
@prefix val: <http://www.w3.org/2001/XMLSchema#> .

val:SteeringWheel a sh:NodeShape ;
    sh:property [ sh:datatype val:string ; sh:name "Angle" ; sh:path car:Angle ],
        [ sh:datatype val:string ; sh:name "VIN" ; sh:path car:vin ] .
"""

def test_fix_shacl_int_datatypes():
    with tempfile.TemporaryDirectory() as tmp:
        raw = Path(tmp) / "raw.ttl"
        raw.write_text(SAMPLE_SHACL_INT)

        graph = Graph()
        graph.parse(raw, format="turtle")
        fix_shacl_int_datatypes(graph)

        car = Namespace("http://example.ontology.com/car#")

        # Angle is a known integer field -> corrected to xsd:integer
        angle_shape = next(graph.subjects(SH.path, car.Angle))
        assert graph.value(angle_shape, SH.datatype) == XSD.integer

        # VIN is not in SHACL_INTEGER_FIELDS -> left untouched as xsd:string
        vin_shape = next(graph.subjects(SH.path, car.vin))
        assert graph.value(vin_shape, SH.datatype) == XSD.string
