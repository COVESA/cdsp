This directory contains files related to the Docker deployment of the Central Data Service Playground.

# Central Data Service Playground
NOTE: The documentation below is a work in progress as we work on the Phase 1 PoC. The intention is to help but it may be incomplete.

FIXME: Document deployment and the basic result for each backend. Suggest following simple template set by https://github.com/docker/awesome-compose as a starting point.
## With Apache IoTDB data store backend
### Deploy with Docker Compose
```
$ sudo docker compose -f docker-compose-waii-iotdb.yml up -d
[+] Running 5/5
 ✔ Network cdsp-waii-iotdb_default   Created                                                                                                             0.1s
 ✔ Container iotdb-service           Started                                                                                                             0.1s
 ✔ Container waii_container_volumes  Started                                                                                                             0.1s
 ✔ Container vissv2server            Started                                                                                                             0.1s
 ✔ Container app_redis               Started
```
### Expected Result
Listing should show three running containers as shown below (fixme: VISS not currently running):
```
$ sudo docker ps
CONTAINER ID   IMAGE                           COMMAND                  CREATED         STATUS         PORTS                                       NAMES
78dd9c624196   redis                           "docker-entrypoint.s…"   2 minutes ago   Up 2 minutes   6379/tcp                                    app_redis
00f36a4a4d62   apache/iotdb:1.2.2-standalone   "/usr/bin/dumb-init …"   2 minutes ago   Up 2 minutes   0.0.0.0:6667->6667/tcp, :::6667->6667/tcp   iotdb-service
```
You can confirm the Apache IoTDB server is running by connecting to it with the CLI client (_quit_ to exit):
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

Stop and remove the containers:
```
$ sudo docker compose -f docker-compose-waii-iotdb.yml down
```

## With Realm data store backend
TBA
## With SQLite or Redis data store backend
TBA
