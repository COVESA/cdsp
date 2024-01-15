This directory contains files related to the Docker deployment of the Central Data Service Playground.

# Central Data Service Playground
NOTE: The documentation below is a work in progress as we work on the Phase 1 PoC. The intention is to help but it may be incomplete.

FIXME: Document deployment and the basic result for each backend. Suggest following simple template set by https://github.com/docker/awesome-compose as a starting point.

## WAII docker build setup
The [WAII VISS Data Server](https://github.com/w3c/automotive-viss2) has no pre-built image in a docker image repository and must therefore be built. Whilst the upstream documentation for WAII is considered the reference documentation for build environment setup this section collects information that we have needed for a successful build of master branch commit bd92da77.

### Install golang
The WAII [build tutorial](https://w3c.github.io/automotive-viss2/build-system/) says to install golang version 1.13 or later.
Go install instructions can be found here: https://go.dev/learn/

Depending on your distro you may need to setup `GOROOT` and `GOPATH`. This was not required on Ubuntu 20 LTS, but was on Mac.

### Create persistent local volume /tmp/docker
The upstream Docker compose assumes the existence of the local directory `/tmp/docker` but does not create it. Fix the issue by creating it yourself:

```
$ mkdir /tmp/docker
```
See upstream issue report https://github.com/w3c/automotive-viss2/issues/99 for details.

### Generate credentials (testCredGen build error)
The upstream Dockerfile assumes that credentials have already been created and will fail to build if they are not found. Generate them by running `./testCredGen.sh ca` from the directory `/cdsp/automotive-viss2/testCredGenRun`

See upstream issue report https://github.com/w3c/automotive-viss2/issues/86 for details.

### Disable Access Grant support (agt_public_key.rsa build error)
There is a current issue with the upstream WAII VISS Server Dockerfile in which the server fails to build due to a missing public key for the Access Control server. See upstream issue report https://github.com/w3c/automotive-viss2/issues/88 for details. After discussion with the upstream maintainers the current workaround is to comment out the relevant following line from the end of the Dockerfile. The change should be made to `cdsp/cdsp/automotive-viss2/Dockerfile`
```
#COPY --from=builder /build/server/agt_server/agt_public_key.rsa .
```

If your project requires Access Grant support please discuss enabling it with the WAII community.

### Mac build error "ERROR [internal] load metadata for docker.io"
On a Mac build errors related to docker metadata such as `ERROR [internal] load metadata for docker.io/library/golang` have been observed. This [serverfault article](https://serverfault.com/a/1131599) suggests commenting the line `"credsStore": "desktop"` from the Docker `config.json` for your user. This was found to work.

## With Apache IoTDB data store backend
### Deploy with Docker Compose
Start the containers:
```
$ sudo docker compose -f docker-compose-waii-iotdb.yml up -d
[+] Running 5/5
 ✔ Network cdsp-waii-iotdb_default   Created                                                                                                             0.1s
 ✔ Container iotdb-service           Started                                                                                                             0.1s
 ✔ Container waii_container_volumes  Started                                                                                                             0.1s
 ✔ Container vissv2server            Started                                                                                                             0.1s
 ✔ Container app_redis               Started
```

Stop and remove the containers:
```
$ sudo docker compose -f docker-compose-waii-iotdb.yml down
```
### Expected Result
Listing should show three running containers as shown below:
```
$ sudo docker ps
CONTAINER ID   IMAGE                           COMMAND                  CREATED          STATUS          PORTS                                                          NAMES
72d793c9619c   cdsp-waii-iotdb-vissv2server    "/app/vissv2server -…"   21 minutes ago   Up 21 minutes   127.0.0.1:8081->8081/tcp, 127.0.0.1:8887-8888->8887-8888/tcp   vissv2server
b9418de22ea5   redis                           "docker-entrypoint.s…"   21 minutes ago   Up 21 minutes   6379/tcp                                                       app_redis
24858f79203b   apache/iotdb:1.2.2-standalone   "/usr/bin/dumb-init …"   21 minutes ago   Up 21 minutes   0.0.0.0:6667->6667/tcp, :::6667->6667/tcp                      iotdb-service
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
#### WAII VISS server
You can confirm the WAII VISS server is running by using one of its included clients. 

The following example uses the javascript HTML client from `automotive-viss2/client/client-1.0/Javascript/httpclient.html`:

1. Get the IP address of the VISS server using docker inspect (in this example 192.168.128.5 is returned.):
```
$ sudo docker inspect -f '{{range.NetworkSettings.Networks}}{{.IPAddress}}{{end}}' vissv2server
192.168.128.5
```

2. In your web browser open the javascript HTML client at `cdsp/cdsp/automotive-viss2/client/client-1.0/Javascript/httpclient.html`. The GUI of the client should be displayed.

3. The client needs to be told where to find the server. Enter the IP address returned in step 1 into the `host IP` text box and press the `Server IP` button.

4. Now lets communicate with the server by requesting a VSS data node:
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

## With Realm data store backend
TBA
## With SQLite or Redis data store backend
TBA
