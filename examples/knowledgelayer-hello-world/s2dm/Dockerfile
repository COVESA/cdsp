FROM ghcr.io/astral-sh/uv:python3.14-bookworm-slim

WORKDIR /app
COPY pyproject.toml uv.lock ./
RUN uv sync --frozen --no-install-project

COPY src ./src
COPY cdsp-custom.graphql naming.yaml ./
RUN uv sync --frozen

RUN useradd --create-home --shell /bin/bash appuser
USER appuser

ENTRYPOINT ["uv", "run", "s2dmc"]
CMD ["--il-out", "/out/il/vss_data_points.yaml", \
     "--kl-rdfox-out", "/out/kl-rdfox/vehicle_shacl.ttl", \
     "--kl-rdf4j-out", "/out/kl-rdf4j/vehicle_shacl.ttl"]
