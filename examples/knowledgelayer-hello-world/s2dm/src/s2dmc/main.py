#!/usr/bin/env python3
import argparse
import subprocess
import sys
from pathlib import Path

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
        "--kl-out",
        type=Path,
        default=EXAMPLES_DIR / "KL-config/shared/shacl/vehicle_shacl.ttl",
        help="Path to save the generated KL SHACL file, shared by all reasoners.",
    )
    return parser.parse_args()

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

    args.il_out.parent.mkdir(parents=True, exist_ok=True)
    run_export([
        "export", "vspec",
        "-s", str(args.schema),
        "--naming-config", str(args.naming_config),
        "-o", str(args.il_out)
    ])

    args.kl_out.parent.mkdir(parents=True, exist_ok=True)
    run_export([
        "export", "shacl",
        "-s", str(args.schema),
        "--naming-config", str(args.naming_config),
        "--shapes-namespace", SHAPES_NAMESPACE,
        "--shapes-namespace-prefix", SHAPES_NAMESPACE_PREFIX,
        "--model-namespace", MODEL_NAMESPACE,
        "--model-namespace-prefix", MODEL_NAMESPACE_PREFIX,
        "-o", str(args.kl_out),
    ])

if __name__ == "__main__":
    main()
