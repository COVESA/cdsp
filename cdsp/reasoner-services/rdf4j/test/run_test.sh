#!/bin/bash
# Manual REST API test for the RDF4J reasoner service.
# Requires the service to be running: docker compose --profile rdf4j up rdf4j-stream-reasoner-service

set -e

BASE_URL="http://localhost:12110"
DATASTORE="test-ds"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "=== 1. Create datastore '$DATASTORE' ==="
curl -sf -X POST "$BASE_URL/datastores/$DATASTORE" && echo "OK" || echo "(already exists)"

echo ""
echo "=== 2. Load SHACL inference rules ==="
curl -sf -X POST "$BASE_URL/datastores/$DATASTORE/rules/add" \
    -H "Content-Type: text/turtle" \
    --data-binary "@$SCRIPT_DIR/inference_rules.shacl"
echo "OK"

echo ""
echo "=== 3. Load RDF data ==="
curl -sf -X POST "$BASE_URL/datastores/$DATASTORE/content" \
    -H "Content-Type: text/turtle" \
    --data-binary "@$SCRIPT_DIR/test_data.ttl"
echo "OK"

echo ""
echo "=== 4. Query inference results ==="
curl -sf -X POST "$BASE_URL/datastores/$DATASTORE/sparql" \
    -H "Content-Type: application/sparql-query" \
    -H "Accept: text/tab-separated-values" \
    --data-binary "@$SCRIPT_DIR/query_results.rq"

echo ""
echo ""
echo "Expected: one HighSpeedObservation for ob_speed_t2 (speed=62, datetime=2026-06-30T09:00:01)"

echo ""
echo "=== 5. xsd:duration arithmetic compatibility check ==="
echo "Testing xsd:dateTime subtraction and xsd:duration comparison..."
DURATION_RESULT=$(curl -sf -X POST "$BASE_URL/datastores/$DATASTORE/sparql" \
    -H "Content-Type: application/sparql-query" \
    -H "Accept: text/tab-separated-values" \
    --data-binary "@$SCRIPT_DIR/query_duration_compat.rq")

ROW_COUNT=$(echo "$DURATION_RESULT" | tail -n +2 | grep -c . || true)

if [ "$ROW_COUNT" -eq 0 ]; then
    echo "RESULT: 0 rows — xsd:dateTime arithmetic is NOT supported (RDF4J/SPARQL 1.1 limitation)."
    echo "        FILTER (?time_diff < \"PT4S\"^^xsd:duration) will silently drop all results."
    echo "        => Remove this FILTER from select_driving_style.rq when using RDF4J."
else
    echo "RESULT: $ROW_COUNT row(s) — xsd:dateTime arithmetic IS supported."
    echo "$DURATION_RESULT"
fi
