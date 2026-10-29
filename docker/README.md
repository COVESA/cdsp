This directory contains files related to the Docker deployment of the Central Data Service Playground.

# Central Data Service Playground

The compose file `docker-compose-cdsp.yml` provides a containerized deployment of the playground using docker.

## Table of contents

- [Docker installation](#docker-installation)
- [VISSR docker image build setup](#vissr-docker-image-build-setup)
- [Websocket-Server (CDSP - information layer) docker image build setup](#websocket-server-cdsp---information-layer-docker-image-build-setup)
- [Websocket-Client (CDSP - knowledge layer) docker image build setup](#websocket-client-cdsp---knowledge-layer-docker-image-build-setup)
  - [RDFox reasoner (`rdfox` profile, licensed)](#rdfox-restful-api)
  - [RDF4J reasoner (`rdf4j` profile, open-source)](#rdf4j-reasoner)
- [Deploy with Docker Compose](#deploy-with-docker-compose)

## Docker installation

If you are not familiar with Docker it is a widely deployed and well documented technology. For which you should find numerous tutorials on its use on the internet.

At the time of writing Docker is available in two different editions with different licenses:

1. [Docker EE/Docker Desktop](https://www.docker.com/products/docker-desktop/) is the commercial edition that provides a GUI and commercial support.
2. [Docker CE (Community Edition)](https://docs.docker.com/engine/) is open source and uses the Docker Engine CLI.

As COVESA operates as an open source community the playground docker support has been developed and documented using the Community Edition and the Docker Engine CLI. The docker compose plugin has been used for development of the compose definitions.

Comprehensive Docker CE [installation instructions](https://docs.docker.com/engine/install/) for different distributions can be found upstream in the official Docker documentation.

If you have a recent version of Docker installed but without the compose plugin, please add the plugin or upgrade.

### Prepare Environment Variables for Docker

To use the provided [docker-compose-cdsp.yml](docker-compose-cdsp.yml) there must be a `.env` file in the same folder. Create it from the template and adjust the values to your setup:

```shell
cp .env.example .env
```

`.env` is not tracked in git, so your values and any secrets stay local. Do not put them into `.env.example`. The variables are explained in the sections of the services that use them: [Information Layer](#configuring-the-env-file) and [Knowledge Layer](#3-websocket-client-cdsp---knowledge-layer).

> [!NOTE]
> The reasoner used by the Knowledge Layer is selected via a Docker Compose [profile](https://docs.docker.com/compose/how-tos/profiles/): `rdfox` (commercial, requires the license above) or `rdf4j` (open-source, no license required). See [Websocket client (CDSP - knowledge layer) docker image build setup](#websocket-client-cdsp---knowledge-layer-docker-image-build-setup). `RDFOX_LIC_PATH` is only needed if you use the `rdfox` profile, but the variable must still exist in `.env` for Docker Compose to parse the file.

### Generated configuration (S2DM)

The Information Layer schema (`vss_data_points.yaml`) and the Knowledge Layer SHACL shapes (`vehicle_shacl.ttl`) are generated from one GraphQL schema by the [S2DM generator](../cdsp/s2dm/README.md). Docker Compose runs it as the one-shot service `s2dm-config-gen` before the Information Layer and the Knowledge Layer start, so nothing extra is needed. The generated files are not tracked in git:

- `cdsp/information-layer/handlers/config/schema-files/vss_data_points.yaml`, mounted into the Information Layer
- `cdsp/knowledge-layer/symbolic-reasoner/examples/use-case/shared/shacl/vehicle_shacl.ttl`, mounted into the Knowledge Layer

After changing the schema, rebuild the generator image so the change is picked up: `docker compose -f docker-compose-cdsp.yml build s2dm-config-gen`.

## VISSR docker image build setup

The [VISSR VISS Data Server](https://github.com/COVESA/vissr) has no pre-built image in a docker image repository and must therefore be built. Whilst the upstream documentation for VISSR is considered the reference documentation for build environment setup this section collects information that we have needed for a successful build of master branch commit 27d6de0.

NOTE: This project intends to work towards the availability of a pre-built VISSR docker image so that the playground docker deployment can be a simple download of images.

If you wish to quickly try the playground without the need for build setup as a first step, this [tip](#tip-i-am-blocked-by-vissr-build-issues-or-do-not-want-to-build) below tells you to how to start the playground without the VISS server.

### Install golang

The VISSR [build tutorial](https://covesa.github.io/vissr/build-system/) says to install golang version 1.13 or later.
Go install instructions can be found here: https://go.dev/learn/

Depending on your distro you may need to setup `GOROOT` and `GOPATH`. This was not required on Ubuntu 20 LTS, but was on Mac.

### Create persistent local volume `/tmp/docker`

The upstream Docker compose assumes the existence of the local directory `/tmp/docker` but does not create it. Fix the issue by creating it yourself:

```shell
$ mkdir /tmp/docker
```

See upstream issue report https://github.com/w3c/automotive-viss2/issues/99 for details.

### Generate credentials (testCredGen build error)

The upstream Dockerfile assumes that credentials have already been created and will fail to build if they are not found. Generate them by running `./testCredGen.sh ca` from the directory `/cdsp/vissr/testCredGenRun`

See upstream issue report https://github.com/w3c/automotive-viss2/issues/86 for details.

### Disable Access Grant support (agt_public_key.rsa build error)

There is a current issue with the upstream VISSR VISS Server Dockerfile in which the server fails to build due to a missing public key for the Access Control server. See upstream issue report https://github.com/w3c/automotive-viss2/issues/88 for details. After discussion with the upstream maintainers the current workaround is to comment out the relevant following line from the end of the `vissv2server` section of the Dockerfile. The change should be made to `cdsp/cdsp/vissr/Dockerfile.rlserver`

```shell
# COPY --from=builder /build/server/agt_server/agt_public_key.rsa .
```

If your project requires Access Grant support please discuss enabling it with the VISSR community.

### Tip: Corporate CA security (download error "tls: failed to verify certificate:")

If you are working behind a corporate security system that places a _man-in-the-middle_ between your host and the internet you may see security errors when artifacts are downloaded as part of the build process.

For example:

```
3.254 client/client-1.0/simple_client.go:27:2: github.com/akamensky/argparse@v1.4.0: Get "https://proxy.golang.org/github.com/akamensky/argparse/@v/v1.4.0.zip": tls: failed to verify certificate: x509: certificate signed by unknown authority
```

This is an attribute of docker, rather than the playground and results when the CA root certificate for the security system is not known.

A solution is to add the CA root certificate to the VISSR Dockerfile which will update the CA store as part of the build and allow the downloads to proceed successfully.

The VISSR Dockerfile already includes an example of this for a Cisco system which uses the CA root certificate `cisco.crt`.

From `cdsp/vissr/Dockerfile.rlserver`:

```dockerfile
# corporate proxy settings can sometimes cause tls verification error. Add root crt to docker container.
COPY testCredGen/cicso-umbrella/cisco.crt /usr/local/share/ca-certificates/cisco.crt
RUN update-ca-certificates
```

To add your own:

1. Ask your IT for the CA root certificate for the security system they use.
2. Copy the `.crt` file into `cdsp/vissr/testCredGen`
3. Add your own line to the Dockerfile to copy the certificate so that it is included when `update-ca-certificates` is run. As shown below.

```dockerfile
COPY testCredGen/cicso-umbrella/cisco.crt /usr/local/share/ca-certificates/cisco.crt
COPY testCredGen/<my company .crt>  /usr/local/share/
RUN update-ca-certificates
```

### Tip: Mac build error "ERROR [internal] load metadata for docker.io"

On a Mac build errors related to docker metadata such as `ERROR [internal] load metadata for docker.io/library/golang` have been observed. This [serverfault article](https://serverfault.com/a/1131599) suggests commenting the line `"credsStore": "desktop"` from the Docker `config.json` for your user. This was found to work.

### Tip: I am blocked by VISSR build issues or do not want to build

You can start working with the playground by starting the deployment with just the Apache IoTDB server which has a pre-built docker image. Its extensive feature set will let you begin experimenting with timeseries data.

In parallel you can work through any VISSR build issues to add VISS support northbound and the other features it supports.

Deploy (start) just IoTDB:

```shell
$ sudo docker compose -f docker-compose-cdsp.yml up -d iotdb-service
# [+] Running 1/0
# ✔ Container iotdb-service  Running                 0.0s
```

## Websocket-Server (CDSP - information layer) docker image build setup

This guide provides instructions for deploying the Information Layer Server using Docker Compose. You can deploy the server only with IoTDB as the backend service for now.

### Prerequisites

- Create `docker/.env` from the template if it is missing, see [Prepare Environment Variables for Docker](#prepare-environment-variables-for-docker). `RDFOX_LIC_PATH` must exist for the [docker compose file](docker-compose-cdsp.yml) to work in general; how to set its value is described at [Knowledge Layer Prerequisites](#prerequisites-1).
- Not only the information layer service but also other services read `docker/.env`. Their variables are described in the service sections below.

### Configuring the `.env` File

The `.env` file contains environment variables that need to be configured depending on the database you are using. The template [.env.example](.env.example) already contains working values for IoTDB. The variables and their defaults are described in the [IoTDB handler README](../cdsp/information-layer/handlers/src/iotdb/README.md).
A custom database handler can be implemented following [database handlers README](../cdsp/information-layer/handlers/src/README.md).

> [!IMPORTANT]
> Before proceeding with the deployment, ensure that the `.env` file is correctly configured for the database you plan to use.

### Using IoTDB

When deploying with IoTDB, ensure that the iotdb-service is up and running before starting the Information Layer Server.

> [!IMPORTANT]
> If required, ensure that the iotdb-service container is started before running the Information Layer Server. You can also configure your own IoTDB connection.

#### Start IoTDB and Information Layer Server

To start both the iotdb-service and information-layer, use the following commands:

```shell
$ sudo docker compose -f docker-compose-cdsp.yml up -d iotdb-service
# [+] Running 1/1
#  ✔ Container iotdb-service         Started

$ sudo docker compose -f docker-compose-cdsp.yml up -d information-layer
# [+] Running 1/1
# ✔ Container information-layer      Started
```

### Expected result

Listing should show two running containers as shown below:

```shell
$ sudo docker ps
```

```
CONTAINER ID   IMAGE                           COMMAND                  CREATED          STATUS          PORTS                                       NAMES
025b5dd05c56   cdsp-information-layer          "docker-entrypoint.s…"   16 minutes ago   Up 16 minutes   0.0.0.0:8080->8080/tcp, :::8080->8080/tcp   information-layer
e16c8ed4ed42   cdsp-iotdb-service              "/usr/bin/dumb-init …"   23 minutes ago   Up 19 minutes   0.0.0.0:6667->6667/tcp, :::6667->6667/tcp   iotdb-service
```

## Websocket client (CDSP - knowledge layer) docker image build setup

The Knowledge Layer needs a symbolic reasoner backend. Two are supported, selected via a Docker Compose profile: [`rdfox`](#rdfox-restful-api) (commercial, requires a license) or [`rdf4j`](#rdf4j-reasoner) (open-source, no license required, default choice for a quick start).

### RDFox RESTful API

In order to get access to `RDFox RESTfull API` it is required to build two Docker images (`rdfox-init` and `rdfox-service`). These services are only started when the `rdfox` profile is selected, e.g. `docker compose -f docker-compose-cdsp.yml --profile rdfox up -d`.

### Prerequisites

- **Running Information Layer Server:** See how to start the information layer [here](#websocket-server-cdsp---information-layer-docker-image-build-setup).
- **RDFox.lic:** this is the license file required by RDFox, containing information that authorizes the use of RDFox, expiry time and usage. **The license file must be provided when running RDFox images to activate the software.**. Its path is set with `RDFOX_LIC_PATH` in `docker/.env`, relative to this folder. The file is generally provided when you acquire a license from [Oxford Semantic Technologies](https://www.oxfordsemantic.tech/).
- The template [.env.example](.env.example) expects the file at `examples/shared-config/rdfox/RDFox.lic` (`RDFOX_LIC_PATH="../examples/shared-config/rdfox/RDFox.lic"`), the same place the hello-world example uses. If you keep it elsewhere, change the path in your `docker/.env`.

#### 1. **Initialization (`rdfox-init` Image)**:

- The `rdfox-init` image is used to set up the RDFox server directory, define roles, and configure persistence.
- It only needs to be run **once** for a fresh setup or if you need to reset the server's configuration (e.g., to initialize roles, passwords, and persistence settings).
- Running this command creates a persistent volume (`rdfox-server-directory`) that stores RDFox's state, including the role configuration, data stores, and settings.

> [!IMPORTANT]
> The image `rdfox-init` need to run only ones to initialize the RDFox server directory.

#### 2. **Daemon (`rdfox-service` Image)**:

- The `rdfox-service` image runs the **RDFox server (daemon)**, which continuously serves requests on port 12110.
- Once initialized, you can start this container (as a daemon) and it will use the `rdfox-server-directory` volume created by the `rdfox-init` command.
- You can restart the daemon multiple times using the same persistent volume, and it will retain all previously initialized settings and data.
- The daemon is initialized with a default role `root` and password `admin`.

> [!WARNING]
> Before build the `rdfox-service`, ensure that the `rdfox-init` were compiled correctly. Only if the logs show successful completion, you can then proceed to start the RDFox daemon.

Use the following commands to start both images:

```shell
$ sudo docker compose -f docker-compose-cdsp.yml --profile rdfox up -d rdfox-init
# ...
# [+] Running 2/2
# ✔ Volume "cdsp_rdfox-server-directory"  created
# ✔ Container rdfox-init                  Started

$ sudo docker compose -f docker-compose-cdsp.yml --profile rdfox up -d rdfox-service
# ...
# [+] Running 2/2
# ✔ Container rdfox-init     Started
# ✔ Container rdfox-service  Started
```

### RDF4J Reasoner

[RDF4J](../cdsp/reasoner-services/rdf4j/README.md) is an open-source reasoner and requires no license, making it the simplest way to try out the Knowledge Layer. It runs as a single service, `rdf4j-stream-reasoner-service`, built from [cdsp/reasoner-services/rdf4j](../cdsp/reasoner-services/rdf4j), and is only started when the `rdf4j` profile is selected:

```shell
$ sudo docker compose -f docker-compose-cdsp.yml --profile rdf4j up -d rdf4j-stream-reasoner-service
# ...
# [+] Running 1/1
# ✔ Container rdf4j-stream-reasoner-service   Started
```

#### 3. **Websocket client (CDSP - knowledge layer)**:

The Knowledge Layer reads its settings from `docker/.env`, created from [.env.example](.env.example). This file is not tracked in git, so keep your own values there. The variables are the same for both profiles; the ones that depend on the profile are set by [docker-compose-cdsp.yml](docker-compose-cdsp.yml).

| Variable                      | Needed | Value for this setup              | Notes                                                                                                                                                                                         |
| ----------------------------- | ------ | --------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `VEHICLE_OBJECT_ID`           | yes    | for example `"VINABCD1234567890"` | Vehicle to subscribe to. Without it the Knowledge Layer stops at start-up.                                                                                                                    |
| `HOST_WEBSOCKET_SERVER`       | yes    | `"information-layer"`             | The default `127.0.0.1` only works when the services run natively, not in containers.                                                                                                         |
| `PORT_WEBSOCKET_SERVER`       | no     | `"8080"`                          | Default `8080`.                                                                                                                                                                               |
| `PORT_REASONER_SERVER`        | no     | `"12110"`                         | Default `12110`.                                                                                                                                                                              |
| `AUTH_REASONER_SERVER_BASE64` | no     | `"cm9vdDphZG1pbg=="`              | Base64 of `root:admin`, the RDFox credentials in the compose file. This is also the default. Not used by RDF4J.                                                                               |
| `REASONER_DATASTORE_NAME`     | no     | `"ds-test"`                       | Default `ds-test`.                                                                                                                                                                            |
| `RDFOX_LIC_PATH`              | yes    | path to `RDFox.lic`               | Only used by the `rdfox` profile, but the variable must exist for Docker Compose to read the file. See [Prepare Environment Variables for Docker](#prepare-environment-variables-for-docker). |

Set by the compose file per profile, so do not put them in `.env` (the compose value wins):

| Variable                   | `rdfox` profile                                     | `rdf4j` profile                                     |
| -------------------------- | --------------------------------------------------- | --------------------------------------------------- |
| `HOST_REASONER_SERVER`     | `rdfox-service`                                     | `rdf4j-stream-reasoner-service`                     |
| `MODEL_CONFIGURATION_PATH` | `/symbolic-reasoner/examples/use-case/rdfox_model/` | `/symbolic-reasoner/examples/use-case/rdf4j_model/` |

The values for this setup are already in [.env.example](.env.example).

At start-up the Knowledge Layer prints the configuration it uses (credentials only as set or not set). If it cannot reach the Information Layer or the reasoner, the error names the host, port and the variables to check. All variables are described in the [Knowledge Layer README](../cdsp/knowledge-layer/README.md#websocket-server-and-reasoner-configuration).

Use the following commands to start the knowledge layer with the reasoner of your choice (the service name matches the profile: `knowledge-layer-rdfox` or `knowledge-layer-rdf4j`):

```shell
$ sudo docker compose -f docker-compose-cdsp.yml --profile rdfox up -d knowledge-layer-rdfox
# ...
# [+] Running 2/2
# ✔ knowledge-layer-rdfox             Created
# ✔ Container knowledge-layer         Started
```

```shell
$ sudo docker compose -f docker-compose-cdsp.yml --profile rdf4j up -d knowledge-layer-rdf4j
# ...
# [+] Running 2/2
# ✔ knowledge-layer-rdf4j             Created
# ✔ Container knowledge-layer         Started
```

The Knowledge Layer services restart automatically when the client exits with an error (`restart: on-failure`). If the Information Layer stops, the Knowledge Layer exits and keeps restarting until the Information Layer is back, then connects and subscribes again. A configuration error (for example a missing `VEHICLE_OBJECT_ID`) also leads to repeated restarts, which do not fix it: the container shows `Restarting` in `docker ps`, `docker logs knowledge-layer` shows the `Error: ...` message, and the fix is to correct `docker/.env`. Messages it cannot process are skipped and logged as `Skipping message: ...`. See [Errors and Exit Codes](../cdsp/knowledge-layer/README.md#errors-and-exit-codes).

#### Tip: Corporate CA security (download error "tls: failed to verify certificate:")

If you are working behind a corporate security system that places a _man-in-the-middle_ between your host and the internet you may see security errors when artifacts are downloaded as part of the build process.

For example:

```
3.254 client/client-1.0/simple_client.go:27:2: github.com/akamensky/argparse@v1.4.0: Get "https://proxy.golang.org/github.com/akamensky/argparse/@v/v1.4.0.zip": tls: failed to verify certificate: x509: certificate signed by unknown authority
```

This is an attribute of docker, rather than the playground and results when the CA root certificate for the security system is not known.

A solution is to add the CA root certificate to the [Knowledge Layer Dockerfile](../cdsp/knowledge-layer/Dockerfile) which will update the CA store as part of the build and allow the downloads to proceed successfully.

The VISSR Dockerfile already includes an example of this for a Cisco system which uses the CA root certificate `cisco.crt`,
[see VISSR Tip: Corporate CA security](#tip-corporate-ca-security-download-error-tls-failed-to-verify-certificate)

To add your own certificate:

1. Ask your IT for the CA root certificate for the security system they use.
2. Copy the `.crt` file into `cdsp/knowledge-layer`
3. Add your own line to the Dockerfile to copy the certificate so that it is included when `update-ca-certificates` is run. As shown below.

```dockerfile
COPY <my company .crt>  /usr/local/share/ca-certificates/<my company .crt>
RUN update-ca-certificates
```

## Deploy All Services with Docker Compose

### Start/stop containers

The reasoner services (`rdfox-init`, `rdfox-service`, `rdf4j-stream-reasoner-service`) and the corresponding `knowledge-layer-rdfox` / `knowledge-layer-rdf4j` service are gated behind the `rdfox` and `rdf4j` [profiles](https://docs.docker.com/compose/how-tos/profiles/), so a profile must be selected with `--profile` to start them.

Start the containers with the open-source RDF4J reasoner:

```shell
$ sudo docker compose -f docker-compose-cdsp.yml --profile rdf4j up -d
# [+] Running 4/5
#  ⠴ Network cdsp_default               Created
#  ✔ Container vissr_container_volumes  Started
#  ✔ Container iotdb-service            Started
#  ✔ Container app_redis                Started
#  ✔ Container vissv2server             Started
#  ✔ Container rdf4j-stream-reasoner-service   Started
#  ✔ Container information-layer        Started
#  ✔ Container knowledge-layer          Started
```

Or with the licensed RDFox reasoner:

```shell
$ sudo docker compose -f docker-compose-cdsp.yml --profile rdfox up -d
# [+] Running 4/5
#  ⠴ Network cdsp_default               Created
#  ✔ Container vissr_container_volumes  Started
#  ✔ Container iotdb-service            Started
#  ✔ Container app_redis                Started
#  ✔ Container vissv2server             Started
#  ✔ Container rdfox-service            Started
#  ✔ Container rdfox-init               Exited
#  ✔ Container information-layer        Started
#  ✔ Container knowledge-layer          Started
#  ✔ Container rdfox-service            Started
```

Sometimes a service might not start up successfully when all are started with a single docker compose command. The reason for this are racing conditions between service startups, that docker has no control over.
An easy fix can be to run the same startup command again.

Stop and remove the containers (use the same `--profile` flag you started with):

```shell
$ sudo docker compose -f docker-compose-cdsp.yml --profile rdf4j down
# [+] Running 5/5
#  ✔ Container vissv2server             Removed
#  ✔ Container app_redis                Removed
#  ✔ Container iotdb-service            Removed
#  ✔ Container vissr_container_volumes  Removed
#  ✔ Container rdf4j-stream-reasoner-service   Removed
#  ✔ Container information-layer        Removed
#  ✔ Container knowledge-layer          Removed
#  ✔ Network cdsp_default               Removed
```

### Expected Result

Listing should show six running containers as shown below (example using the `rdfox` profile):

```shell
$ sudo docker ps


CONTAINER ID   IMAGE                         COMMAND                  CREATED          STATUS                    PORTS                                                                                              NAMES
7829813bdcb8   cdsp-vissv2server             "/app/vissv2server -…"   11 minutes ago   Up 11 minutes             0.0.0.0:8081->8081/tcp, 0.0.0.0:8600->8600/tcp, 0.0.0.0:8887->8887/tcp, 127.0.0.1:8888->8888/tcp   vissv2server
8e21a556e398   redis                         "docker-entrypoint.s…"   11 minutes ago   Up 11 minutes             6379/tcp                                                                                           app_redis
37b461fcdc8f   cdsp-knowledge-layer          "/src/build/bin/reas…"   11 minutes ago   Up 11 minutes                                                                                                                knowledge-layer
92c4eef4df32   oxfordsemantic/rdfox:latest   "/opt/RDFox/RDFox da…"   11 minutes ago   Up 11 minutes             0.0.0.0:12110->12110/tcp, [::]:12110->12110/tcp                                                    rdfox-service
6391e41171a5   cdsp-iotdb-service            "/usr/bin/dumb-init …"   11 minutes ago   Up 11 minutes (healthy)   0.0.0.0:6667->6667/tcp, [::]:6667->6667/tcp                                                        iotdb-service
3af122904fef   cdsp-information-layer        "docker-entrypoint.s…"   11 minutes ago   Up 11 minutes             0.0.0.0:8080->8080/tcp, [::]:8080->8080/tcp                                                        information-layer

```

#### Apache IoTDB

You can confirm the Apache IoTDB server is running by connecting to it with the IoTDB CLI client (_quit_ to exit the client):

```shell
$ sudo docker exec -ti iotdb-service /iotdb/sbin/start-cli.sh -h iotdb-service
```

```
---------------------
Starting IoTDB Cli
---------------------
 _____       _________  ______   ______
|_   _|     |  _   _  ||_   _ `.|_   _ \
  | |   .--.|_/ | | \_|  | | `. \ | |_) |
  | | / .'`\ \  | |      | |  | | |  __'.
 _| |_| \__. | _| |_    _| |_.' /_| |__) |
|_____|'.__.' |_____|  |______.'|_______/  version 2.0.8 (Build: b88b5dc)


Successfully login at iotdb-service:6667
IoTDB>
```

#### VISSR VISS server

You can confirm the VISSR VISS server is running by using one of its included clients.

The following example uses the javascript HTML client from `vissr/client/client-1.0/Javascript/httpclient.html`:

1. In your web browser open the javascript HTML client at `cdsp/cdsp/automotive-viss2/client/client-1.0/Javascript/httpclient.html`. The GUI of the client should be displayed.

2. The client needs to be told where to find the server. The default value of `localhost` in the `host IP` text box should be sufficient, but you can also enter the numerical IP address if you wish, now press the `Server IP` button.

3. Now lets communicate with the server by requesting a VSS data node:

- To the left of the `GET` button enter `Vehicle.Speed` in the `url path` text box.
- Now press the `GET` button to request the query from the server.
- You should see messages returned by the server confirming the request, but the data value returned will be "Data-not-found" if the database contains no values.

```
  Server: readyState=1, status=0
  Server: readyState=2, status=200
  Server: readyState=3, status=200
  Server: {"data":{"dp":{"ts":"2024-01-10T14:56:48Z","value":"Data-not-found"},"path":"Vehicle.Speed"},"ts":"2024-01-10T14:56:48Z"}
```
