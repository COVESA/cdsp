#!/bin/bash
# Manual REST API test for the CQELS reasoner service.
# Requires the service to be running: docker compose -f docker/docker-compose-cdsp.yml --profile cqels up -d cqels-reasoner-service

set -e

BASE_URL="http://localhost:12110"
DATASTORE="test-ds"

if ! curl -sf "$BASE_URL/health" >/dev/null; then
    echo "The CQELS service is not reachable at $BASE_URL. Start it first:"
    echo "  docker compose -f docker/docker-compose-cdsp.yml --profile cqels up -d --build cqels-reasoner-service"
    exit 1
fi

echo "=== 1. Create datastore '$DATASTORE' ==="
curl -sSf -X POST "$BASE_URL/datastores/$DATASTORE"
echo ""

echo ""
QUERY='PREFIX ex: <http://ex.org/>
REGISTER QUERY Speed AS
SELECT ?car ?speed
FROM STREAM Signals [RANGE 5s]
WHERE { STREAM Signals { ?car ex:speed ?speed } }'

echo "=== 2. Register a continuous query ==="
echo "$QUERY"
QUERY_ID=$(curl -sSf -X POST "$BASE_URL/datastores/$DATASTORE/queries" \
    -H "Content-Type: text/plain" \
    --data-binary "$QUERY")
echo "Query id: $QUERY_ID"

# Pushes one stream element to the stream Signals and prints it.
push() {
    echo "eventTime=$1  $2"
    curl -sSf -X POST "$BASE_URL/datastores/$DATASTORE/streams/Signals?eventTime=$1" \
        -H "Content-Type: text/turtle" \
        --data-binary "$2"
    echo ""
}

echo ""
echo "=== 3. Push two stream elements to Signals ==="
push "2026-10-30T10:00:00Z" "<http://ex.org/car1> <http://ex.org/speed> 100 ."
push "2026-10-30T10:00:01Z" "<http://ex.org/car1> <http://ex.org/speed> 120 ."

echo ""
echo "=== 4. Fetch the results (after the reorder tolerance) ==="
sleep 1
curl -sSf "$BASE_URL/datastores/$DATASTORE/queries/$QUERY_ID/results"
echo ""
echo ""
echo "Expected: two rows for car1, speed 100 and speed 120"

echo ""
echo "=== 5. Delete datastore '$DATASTORE' ==="
curl -sSf -X DELETE "$BASE_URL/datastores/$DATASTORE"
echo ""
