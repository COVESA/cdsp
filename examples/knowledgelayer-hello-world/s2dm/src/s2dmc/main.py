#!/usr/bin/env python3
import argparse
import subprocess
import sys
import tempfile
from pathlib import Path
from ruamel.yaml import YAML
from rdflib import Graph, Literal
from rdflib.namespace import SH, XSD

# Workaround for an S2DM issue: its PascalCase converter treats acronyms as regular
# words (Ai/Vin/PwfStatus instead of AI/VIN/PWFStatus). Remove fix_vspec_acronyms
# and fix_shacl_acronyms below once S2DM's own PascalCase conversion handles acronyms.
ACRONYM_FILES = {"Ai": "AI", "Vin": "VIN", "PwfStatus": "PWFStatus"}

# Workaround for an S2DM issue: its SHACL exporter doesn't map the Int8/UInt8/Int16/
# UInt16 scalars (used in cdsp-custom.graphql for fields IL requires a specific
# width for) to an XSD type, so it falls back to xsd:string. These fields are all
# real integers on the vspec side (S2DM maps those scalars correctly there). Remove
# fix_shacl_int_datatypes below once S2DM's SHACL exporter maps these scalars to
# xsd:integer itself.
SHACL_INTEGER_FIELDS = {"Angle", "NominalVoltage", "CurrentGear", "LightIntensity", "WindowPosition"}

PACKAGE_DIR = Path(__file__).parent.resolve()
PROJECT_DIR = PACKAGE_DIR.parent.parent
EXAMPLES_DIR = PROJECT_DIR.parent

# Fixed values, not user-configurable: must match the car:/val: namespace prefixes the
# KL rules and SPARQL queries expect.
SHAPES_NAMESPACE = "http://www.w3.org/2001/XMLSchema#"
SHAPES_NAMESPACE_PREFIX = "val"
MODEL_NAMESPACE = "http://example.ontology.com/car#"
MODEL_NAMESPACE_PREFIX = "car"


def parse_args():
    parser = argparse.ArgumentParser(
        description="Generate IL/KL config files from a GraphQL S2DM schema."
    )
    parser.add_argument(
        "--schema",
        type=Path,
        default=PROJECT_DIR / "cdsp-custom.graphql",
        help="Path to the GraphQL schema file.",
    )
    parser.add_argument(
        "--naming-config",
        type=Path,
        default=PROJECT_DIR / "naming.yaml",
        help="Path to the naming configuration file.",
    )
    parser.add_argument(
        "--il-out",
        type=Path,
        default=EXAMPLES_DIR / "IL-config/schema-files/vss_data_points.yaml",
        help="Path to save the generated IL config file.",
    )
    parser.add_argument(
        "--kl-rdfox-out",
        type=Path,
        default=EXAMPLES_DIR / "KL-config/rdfox_model/shacl/vehicle_shacl.ttl",
        help="Path to save the generated KL SHACL file for RDFox.",
    )
    parser.add_argument(
        "--kl-rdf4j-out",
        type=Path,
        default=EXAMPLES_DIR / "KL-config/rdf4j_model/shacl/vehicle_shacl.ttl",
        help="Path to save the generated KL SHACL file for RDF4J.",
    )
    return parser.parse_args()

# Workaround for S2DM's acronym-casing bug (see ACRONYM_FILES above).
def fix_vspec_acronyms(raw_path, out_path):
    yaml = YAML()
    yaml.preserve_quotes = True
    with raw_path.open("r") as f:
        data = yaml.load(f)

    for idx, key in enumerate(list(data.keys())):
        fixed_segments = [ACRONYM_FILES.get(seg, seg) for seg in key.split(".")]
        if fixed_segments != key.split("."):
            value = data.pop(key)
            data.insert(idx, ".".join(fixed_segments), value)

    out_path.parent.mkdir(parents=True, exist_ok=True)
    with out_path.open("w") as f:
        yaml.dump(data, f)

# Workaround for S2DM's acronym-casing bug (see ACRONYM_FILES above).
def fix_shacl_acronyms(raw_path):
    graph = Graph()
    graph.parse(raw_path, format="turtle")

    for s, p, o in list(graph.triples((None, SH.name, None))):
        fixed = ACRONYM_FILES.get(str(o))
        if fixed:
            graph.remove((s, p, o))
            graph.add((s, p, Literal(fixed)))

    return graph

# Workaround for S2DM's SHACL scalar-to-XSD mapping gap (see SHACL_INTEGER_FIELDS above).
def fix_shacl_int_datatypes(graph):
    for s, _, name in list(graph.triples((None, SH.name, None))):
        if str(name) not in SHACL_INTEGER_FIELDS:
            continue
        for _, p, dtype in list(graph.triples((s, SH.datatype, None))):
            if dtype == XSD.string:
                graph.remove((s, p, dtype))
                graph.add((s, p, XSD.integer))

    return graph

def write_shacl(graph, *out_paths):
    serialized = graph.serialize(format="turtle")
    for out_path in out_paths:
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(serialized)

def run_export(subcommand_args):
    cmd = ["s2dm"] + subcommand_args
    try:
        subprocess.run(cmd, capture_output=True, text=True, check=True)
    except subprocess.CalledProcessError as e:
        print(f"s2dm command failed: {' '.join(cmd)}", file=sys.stderr)
        print(e.stdout, file=sys.stderr)
        print(e.stderr, file=sys.stderr)
        sys.exit(e.returncode)

def main():
    args = parse_args()

    with tempfile.TemporaryDirectory() as temp_dir:
        raw_vspec = Path(temp_dir) / "raw_vspec.yaml"
        raw_shacl = Path(temp_dir) / "raw_shacl.ttl"

        run_export([
            "export", "vspec",
            "-s", str(args.schema),
            "--naming-config", str(args.naming_config),
            "-o", str(raw_vspec)
        ])

        run_export([
            "export", "shacl",
            "-s", str(args.schema),
            "--naming-config", str(args.naming_config),
            "--shapes-namespace", SHAPES_NAMESPACE,
            "--shapes-namespace-prefix", SHAPES_NAMESPACE_PREFIX,
            "--model-namespace", MODEL_NAMESPACE,
            "--model-namespace-prefix", MODEL_NAMESPACE_PREFIX,
            "-o", str(raw_shacl),
        ])

        fix_vspec_acronyms(raw_vspec, args.il_out)
        fixed_shacl_graph = fix_shacl_acronyms(raw_shacl)
        fix_shacl_int_datatypes(fixed_shacl_graph)
        write_shacl(fixed_shacl_graph, args.kl_rdfox_out, args.kl_rdf4j_out)

if __name__ == "__main__":
    main()
