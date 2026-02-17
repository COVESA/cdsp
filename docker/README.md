This directory contains files related to the Docker deployment of the Central Data Service Playground.

# Central Data Service Playground
The compose file `docker-compose-cdsp.yml` provides a containerized deployment of the playground using docker.

## Table of contents
- [Docker installation](#docker-installation)
- [VISSR docker image build setup](#vissr-docker-image-build-setup)
- [Websocket-Server (CDSP - information layer) docker image build setup](#websocket-server-cdsp---information-layer-docker-image-build-setup)
- [Websocket-Client (CDSP - knowledge layer) docker image build setup](#websocket-client-cdsp---knowledge-layer-docker-image-build-setup)
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
To use the provided [docker-compose-cdsp.yml](docker-compose-cdsp.yml) there must be a `.env` file in the same folder. 
Create it and add a placeholder for the RDFox license file for now, like this:
```
RDFOX_LIC_PATH="."  # set path to RDFox.lic file
```


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

- Create a `.env` file in this folder (docker/.env) if it is missing and add for now the variable `RDFOX_LIC_PATH="."  # set path to RDFox.lic file`. this environment variable needs to exist for the [docker compose file](docker-compose-cdsp.yml) to work in general. 
How to set the correct value of this variable is described later at [Knowledge Layer Prerequisites](#prerequisites-1)
- Not only information layer service but also others services will expect the `.env` file to exist and some environment variables to be set. More details can be found in other service readme sections.

### Configuring the `.env` File

The `.env` file contains environment variables that need to be configured depending on the database you are using. Detailed instructions on how to configure the `.env` file for the included database can be found in the [IotDB handlers README](../cdsp/information-layer/handlers/src/iotdb/README.md).
A custom database handler can be implemented following [database handlers README](../cdsp/information-layer/handlers/src/README.md).

> [!IMPORTANT] 
> Before proceeding with the deployment, ensure that the `.env` file is correctly configured for the database you plan to use.

### Using IoTDB

When deploying with IoTDB, ensure that the iotdb-service is up and running before starting the Information Layer Server.

> [!IMPORTANT] 
>  If required, ensure that the iotdb-service container is started before running the Information Layer Server. You can also configure your own IoTDB connection.

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

### RDFox RESTful API

In order to get access to `RDFox RESTfull API` it is required to build two Docker images (`rdfox-init` and `rdfox-service`).

### Prerequisites

- **Running Information Layer Server:** See how to start the information layer [here](#websocket-server-cdsp---information-layer-docker-image-build-setup).
- **RDFox.lic:** this is the license file required by RDFox, containing information that authorizes the use of RDFox, expiry time and usage. **The license file must be provided when running RDFox images to activate the software.**. The relative path of the file should be provided using the environment variable `RDFOX_LIC_PATH="<your_path>/RDFox.lic"` in the `.env` file in this folder. The file is generally provided when you acquire a license from [Oxford Semantic Technologies](https://www.oxfordsemantic.tech/).
-  For example, you could create here a new folder `rdfox` and place `RDFox.lic` in it. 
Then set the environment variable in `.env` to `RDFOX_LIC_PATH="./rdfox/RDFox.lic"`

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
$ sudo docker compose -f docker-compose-cdsp.yml up -d rdfox-init
# ...
# [+] Running 2/2
# ✔ Volume "cdsp_rdfox-server-directory"  created
# ✔ Container rdfox-init                  Started

$ sudo docker compose -f docker-compose-cdsp.yml up -d rdfox-service 
# ...
# [+] Running 2/2
# ✔ Container rdfox-init     Started
# ✔ Container rdfox-service  Started 
```

#### 3. **Websocket client (CDSP - knowledge layer)**:
- Knowledge Layer configuration needs to be added to the `.env` file, like described at [Knowledge Layer Readme](../cdsp/knowledge-layer/README.md#websocket-server-and-rdfox-configuration)
- Keep in mind that the default values for **HOST_WEBSOCKET_SERVER** and **HOST_REASONER_SERVER** are designed for accessing all services running natively on the same machine. But this guide focuses on running all services in separate docker containers, so the host variables needs to reflect the container names. In this case the environment variables should look like this:
```text
# schema REQUIRED_VSS_DATA_POINTS_FILE is the TXT file containing the required data points
REQUIRED_VSS_DATA_POINTS_FILE=vss_data_required.txt

HOST_WEBSOCKET_SERVER="information-layer"
PORT_WEBSOCKET_SERVER="8080"
HOST_REASONER_SERVER="rdfox-service"
PORT_REASONER_SERVER="12110"
# The following are the default Base64-encoded string of the RDFox credentials configured in the /docker/docker-compose-cdsp.yml file
# The credentials are: root:admin
AUTH_REASONER_SERVER_BASE64="cm9vdDphZG1pbg=="
REASONER_DATASTORE_NAME="ds-test"
VEHICLE_OBJECT_ID="VINABCD1234567890" 
```

Use the following commands to start knowledge layer:

```shell
$ sudo docker compose -f docker-compose-cdsp.yml up -d knowledge-layer
# ...
# [+] Running 2/2
# ✔ knowledge-layer             Created 
# ✔ Container knowledge-layer   Started
```

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
Start the containers:
```shell
$ sudo docker compose -f docker-compose-cdsp.yml up -d
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

Stop and remove the containers:
```shell
$ sudo docker compose -f docker-compose-cdsp.yml down
# [+] Running 5/5
#  ✔ Container vissv2server             Removed
#  ✔ Container app_redis                Removed
#  ✔ Container iotdb-service            Removed
#  ✔ Container vissr_container_volumes  Removed
#  ✔ Container rdfox-service            Removed
#  ✔ Container rdfox-init               Removed
#  ✔ Container information-layer        Removed
#  ✔ Container knowledge-layer          Removed
#  ✔ Container rdfox-service            Removed
#  ✔ Network cdsp_default               Removed
```
### Expected Result
Listing should show three running containers as shown below:
```shell
$ sudo docker ps


CONTAINER ID   IMAGE                         COMMAND                  CREATED          STATUS                    PORTS                                                                                              NAMES
f43ed0c6ba0a   cdsp-iotdb-service            "/usr/bin/dumb-init …"   11 minutes ago   Up 10 minutes             0.0.0.0:6667->6667/tcp, :::6667->6667/tcp                                                          iotdb-service
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
|_____|'.__.' |_____|  |______.'|_______/  version 1.2.2 (Build: 5d0bfb0)


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
