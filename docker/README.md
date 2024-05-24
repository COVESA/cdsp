This directory contains files related to the Docker deployment of the Central Data Service Playground.

# Central Data Service Playground
The compose file `docker-compose-cdsp.yml` provides a containerized deployment of the playground using docker.

## Docker installation
If you are not familiar with Docker it is a widely deployed and well documented technology. For which you should find numerous tutorials on its use on the internet.

At the time of writing Docker is available in two different editions with different licenses:
1. [Docker EE/Docker Desktop](https://www.docker.com/products/docker-desktop/) is the commercial edition that provides a GUI and commercial support.
2. [Docker CE (Community Edition)](https://docs.docker.com/engine/) is open source and uses the Docker Engine CLI.

As COVESA operates as an open source community the playground docker support has been developed and documented using the Community Edition and the Docker Engine CLI. The docker compose plugin has been used for development of the compose definitions.

Comprehensive Docker CE [installation instructions](https://docs.docker.com/engine/install/) for different distributions can be found upstream in the official Docker documentation.

If you have a recent version of Docker installed but without the compose plugin, please add the plugin or upgrade.

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

```
$ mkdir /tmp/docker
```
See upstream issue report https://github.com/w3c/automotive-viss2/issues/99 for details.

### Generate credentials (testCredGen build error)
The upstream Dockerfile assumes that credentials have already been created and will fail to build if they are not found. Generate them by running `./testCredGen.sh ca` from the directory `/cdsp/vissr/testCredGenRun`

See upstream issue report https://github.com/w3c/automotive-viss2/issues/86 for details.

### Disable Access Grant support (agt_public_key.rsa build error)
There is a current issue with the upstream VISSR VISS Server Dockerfile in which the server fails to build due to a missing public key for the Access Control server. See upstream issue report https://github.com/w3c/automotive-viss2/issues/88 for details. After discussion with the upstream maintainers the current workaround is to comment out the relevant following line from the end of the `vissv2server` section of the Dockerfile. The change should be made to `cdsp/cdsp/vissr/Dockerfile.rlserver`
```
#COPY --from=builder /build/server/agt_server/agt_public_key.rsa .
```

If your project requires Access Grant support please discuss enabling it with the VISSR community.

### Generate `vss_vissv2.binary`
The VISSR server component requires a file called `vss_vissv2.binary` to understand the VSS tree it must work with. Unfortunately, VISSR provides no default file and you must therefore generate it yourself.

Instructions for doing that can be found in the VISSR documentation site [here](https://covesa.github.io/vissr/server/#vss-tree-configuration)

Tip: The playground maintainers have found that the method involving running `make binary` in a git clone of the VSS source tree to generate the file is straight forward. Note: check the VSS readme for the python requirements for the tooling.

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
```
#corporate proxy settings can sometimes cause tls verification error. Add root crt to docker container.
COPY testCredGen/cicso-umbrella/cisco.crt /usr/local/share/ca-certificates/cisco.crt
RUN update-ca-certificates
```
To add your own:
1. Ask your IT for the CA root certificate for the security system they use.
2. Copy the `.crt` file into `cdsp/vissr/testCredGen`
3. Add your own line to the Dockerfile to copy the certificate so that it is included when `update-ca-certificates` is run. As shown below.

```
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
```
$ sudo docker compose -f docker-compose-cdsp.yml up -d iotdb-service
[+] Running 1/0
 ✔ Container iotdb-service  Running                 0.0s
```

## Deploy with Docker Compose
### Start/stop containers
Start the containers:
```
$ sudo docker compose -f docker-compose-cdsp.yml up -d
[+] Running 4/5
 ⠴ Network cdsp_default               Created                 1.5s
 ✔ Container vissr_container_volumes  Started                 0.5s
 ✔ Container iotdb-service            Started                 0.7s
 ✔ Container app_redis                Started                 1.0s
 ✔ Container vissv2server             Started                 1.3s
```

Stop and remove the containers:
```
$ sudo docker compose -f docker-compose-cdsp.yml down
[+] Running 5/5
 ✔ Container vissv2server             Removed                 0.0s
 ✔ Container app_redis                Removed                 0.3s
 ✔ Container iotdb-service            Removed                 2.2s
 ✔ Container vissr_container_volumes  Removed                 0.0s
 ✔ Network cdsp_default               Removed                 0.1s
```
### Expected Result
Listing should show three running containers as shown below:
```
$ sudo docker ps
NAME            IMAGE                           COMMAND                  SERVICE         CREATED          STATUS          PORTS
app_redis       redis                           "docker-entrypoint.s…"   redis           10 minutes ago   Up 10 minutes   6379/tcp
iotdb-service   apache/iotdb:1.2.2-standalone   "/usr/bin/dumb-init …"   iotdb-service   10 minutes ago   Up 10 minutes   0.0.0.0:6667->6667/tcp, :::6667->6667/tcp
vissv2server    cdsp-vissv2server               "/app/vissv2server -…"   vissv2server    10 minutes ago   Up 4 seconds    0.0.0.0:8081->8081/tcp, 0.0.0.0:8600->8600/tcp, 0.0.0.0:8887->8887/tcp, 127.0.0.1:8888->8888/tcp
```
#### Apache IoTDB
You can confirm the Apache IoTDB server is running by connecting to it with the IoTDB CLI client (_quit_ to exit the client):
```
$ sudo docker exec -ti iotdb-service /iotdb/sbin/start-cli.sh -h iotdb-service
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

You can query what VSS nodes the server understands by asking for the VSS path list using the URL `http://localhost:8081/vsspathlist`. Entering that URL in your web browser will typically give you a graphical rendering of the JSON data returned.