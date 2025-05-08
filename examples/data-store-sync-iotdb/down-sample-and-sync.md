
# Automatically streaming/synchronising VSS data between two data stores
These are basic notes to demonstrate using the example docker compose to sync data between two Apache IoTDB instances.

System context:
+ Data store 1 is Vehicle HPC DB.
+ Data store 2 is Cloud DB.

Apache IoTDB v1.3.3 Data Sync documentation can be found [here](https://iotdb.apache.org/UserGuide/V1.3.x/User-Manual/Data-Sync_apache.html)

## Setup
1. Start the Playground docker environment

Start two docker services `vehicle-hpc-iotdb-service` and `cloud-iotdb-service` each of which contain a (standalone) Apache IoTDB instance.
```
docker compose up -d
```

2. Connect to IoTDB client in HPC container

Open new terminal then:
```
docker exec -ti hpc-iotdb-service /iotdb/sbin/start-cli.sh -h hpc-iotdb-service
```

3. Connect to IoTDB client in Cloud container

Open new terminal then:
```
docker exec -ti cloud-iotdb-service /iotdb/sbin/start-cli.sh -h cloud-iotdb-service
```

## Create some data to sync in HPC DB
Here I reuse data from the CDSP down-sample example, but you can use your own.

1. Create database in HPC DB

In the IoTDB client in the HPC container:
```sql
create database root.test2
```
2. Copy dataset into HPC DB volume for importing

From host terminal:
```
sudo cp ../vehicle-speed-downsample-iotdb/vehicle_speed_rl_dataset.csv hpc-iotdb-data/
```
3. Import data into HPC

From host terminal:
```
docker exec -ti hpc-iotdb-service /iotdb/tools/import-data.sh -h hpc-iotdb-service -p 6667 -u root -pw root -s /iotdb/data/vehicle_speed_rl_dataset.csv
```
Data will be in the timeseries `` root.test2.vin123test.`Vehicle.Speed` ``

4. (Optional) Downsample the data in HPC DB into new timeseries

**Setup**: The down-sample tutorial uses the DB function `sample` from the optional IoTDB Data Quality library. To be able to call it we must do a one time registration of the library functions. This can be done from the host terminal by executing the supplied script (detail in the tutorial [here](https://github.com/COVESA/cdsp/tree/main/examples/vehicle-speed-downsample-iotdb#data-quality-library-setup)):
```
docker exec -ti hpc-iotdb-service /iotdb/sbin/register-UDF.sh
```

**Down-sample**: In HPC client down sample the `Vehicle.Speed` data into a new timeseries `root.test2.vin123test.speed_upload` as if we were doing data reduction:
```sql
select sample(`Vehicle.Speed`,'method'='triangle','k'='100') into root.test2.vin123test(speed_upload) from root.test2.vin123test
```

## HPC to Cloud sync

### Sync everything
1. Create pipe on HPC side towards Cloud DB

In HPC client:
```sql
create pipe V2C
WITH SOURCE (
  'source'= 'iotdb-source',
  'realtime.mode' = 'stream'
) 
with SINK (
  'sink'='iotdb-thrift-async-sink',
  'node-urls' = 'cloud-iotdb-service:6667',
)
```
Pipe will execute immediately.

Confirm the timeseries in the DB arrived in the Cloud DB CLI:
```
show timeseries
```


### Sync only relevant data
Emulate the syncing only of relevant data by first processing in-vehicle and only synching the result. This also emulates the concept of data reduction for reduced transmission costs and network traffic.

What timeseries are synched is controlled using the `path` variable of the `source` plugin in the `pipe`. See IoTDB documentation for details, e.g. wildcards.

1. Create pipe on HPC DB to only sync data prepared for upload in the timeseries
`root.test2.vin123test.speed_upload`

In the HPC DB Client:
```sql
create pipe V2CPartial
WITH SOURCE (
  'source'= 'iotdb-source',
  'realtime.mode' = 'stream',
  'path'='root.test2.vin123test.speed_upload'
) 
with SINK (
  'sink'='iotdb-thrift-async-sink',
  'node-urls' = 'cloud-iotdb-service:6667',
)
```

Confirm the timeseries arrived in the Cloud DB client:
```sql
select count(speed_upload) from root.**
```
### Pipe admin

Stop pipe:
```
stop pipe <pipe ID>
```

Start pipe:
```
start pipe <pipe ID>
```

Show pipes:
```
show pipes
```