# RemotiveLabs Bridge/Feeder
This a simple example of using the [RemotiveLabs virtual signal platform](https://remotivelabs.com/) as a feeder of vehicle data into the playground [Apache IoTDB](https://covesa.github.io/cdsp/manuals/apache-iotdb/) data store.

It is implemented as a bridge between an RemotiveLabs Broker as the data source and the IoTDB server or the Information Layer Server. You specify the signals you wish to subscribe too. The bridge waits for the subscribed signals to be sent from the
RemotiveLabs broker, formats them and then writes them 
* to IoTDB using the specified IoTDB path or
* the websocket API on the Information Layer Server. Depending on the server configuration the data is written to IoTDB or RealmDB.

For IoTDB the code assumes the timeseries into which the data will be written already exists in IoTDB. See the setup section for an example.


## Setup
### IoTDB schema
The bridge assumes that the timeseries you are writing data into already exists in IoTDB.

The following is an IoTDB SQL example for creating such a timeseries schema that uses all the supported VSS signals in the RemotiveLabs dataset example `Night drive to Luftkastellet`:

1. Create database
```
CREATE DATABASE root.test2
```
2. Create timeseries
```
CREATE ALIGNED TIMESERIES root.test2.dev1(`Vehicle.Speed` FLOAT, `Vehicle.Chassis.Accelerator.PedalPosition` INT32, `Vehicle.Powertrain.Transmission.CurrentGear` INT32, `Vehicle.Powertrain.TractionBattery.NominalVoltage` INT32, `Vehicle.Powertrain.TractionBattery.StateOfCharge.CurrentEnergy` FLOAT, `Vehicle.Chassis.SteeringWheel.Angle` INT32, `Vehicle.CurrentLocation.Longitude` DOUBLE, `Vehicle.CurrentLocation.Latitude` DOUBLE, `Vehicle.CurrentLocation.HorizontalAccuracy` DOUBLE)
```
### RealmDB schema
Is to be clarified

### Bridge install
The bridge requires Python3. A list of required packages is provided that can be installed with the pip package manager.

Install the python packages used by the bridge:

    pip3 install -r requirements.txt

### RemotiveLabs
The RemotiveLabs platform has extensive documentation so we will not reproduce it here.

This example assumes you have access to a RemotiveLabs Broker and you know its URL and API Key or Token.

RemotiveLabs have both a Cloud Demo and a Free Tier with pre-recorded datasets that can be used as a starting point.

## Usage

### Configuration
Passing the `-h` parameter to the bridge will display the command line options:
```
python3 rl-bridge.py -h
usage: rl-bridge.py [-h] [-u URL] [-x X_API_KEY] [-t ACCESS_TOKEN] -s [SIGNALS ...] -o {iotdb,information-layer} [-i ID]

Provide address to RemotiveBroker

options:
  -h, --help            show this help message and exit
  -u, --url URL         URL of the RemotiveBroker
  -x, --x_api_key X_API_KEY
                        API key is required when accessing brokers running in the cloud
  -t, --access_token ACCESS_TOKEN
                        Personal or service-account access token
  -s, --signals [SIGNALS ...]
                        Signal to subscribe to
  -o, --output_mode {iotdb,information-layer}
                        Output sent to iotdb or information-layer
  -i, --id ID           ID to which signals are related to (only information-layer mode)
  -ilu, --information_layer_url INFORMATION_LAYER_URL
                        URL of the information layer. If not provided ws://localhost:8080 will be used.

```
Tip: To avoid displaying RemotiveLabs Broker secrets on the command line you can pass them using export variables, e.g:
```
export RL_BROKER_URL=<broker URL>
export RL_API_KEY=<key>

python3 rl-bridge.py --url $RL_BROKER_URL --x_api_key $RL_API_KEY
```
The bridge needs to be configured so it knows where to find the RemotiveLabs Broker and the Apache IoTDB database server and how to make the connections.

For the RemotiveLabs Broker this is at a minimum the URL of the Broker and either the API Key or Token to access it. These values are passed as parameters as shown above.

The IoTDB connection is configured as global variables in the python source:
```
ip = "127.0.0.1"
port_ = "6667"
username_ = "root"
password_ = "root"
session = Session(ip, port_, username_, password_, fetch_size=1024, zone_id="GMT+00:00")
device_id_ = "root.test2.dev1"
```
| Variable | Type | Description |
| --- | --- | --- 
|`ip`|String|Hostname or IP address of the IoTDB server|
|`port_`|String|RPC port of the server. Default is 6667|
|`username_`|String|Username to access the server. Default is root|
|`password_`|String|Password to access the server. Default is root|
|`zone_id`|String| Time zone of the connection
|`device_id_`|String|Timeseries prefix path of where to write the received data in the database|

For a standard build of the playground with an unchanged IoTDB server configuration you would typically not need to change the url, port and user account details.

More typically you would change `device_id_` if needed to reflect how you are organizing data in IoTDB.

For sending data to the information-layer the websocket server has to listen on `ws://localhost:8080`.

### Signal subscription examples
#### Single signal to IoTDB
Execute the bridge and subscribe to the signal `Vehicle.Speed` in the namespace `vss`:
```
python3 rl-bridge.py --url $RL_BROKER_URL --x_api_key $RL_API_KEY -o iotdb --signals vss:Vehicle.Speed
```

#### Multiple signals to IoTDB
Execute the bridge and subscribe to all supported VSS signals in the RemotiveLabs dataset example `Night drive to Luftkastellet`:
```
python3 rl-bridge.py --url $RL_BROKER_URL --x_api_key $RL_API_KEY -o iotdb --signals vss:Vehicle.Speed vss:Vehicle.Chassis.Accelerator.PedalPosition vss:Vehicle.Powertrain.Transmission.CurrentGear vss:Vehicle.Powertrain.TractionBattery.NominalVoltage vss:Vehicle.Powertrain.TractionBattery.StateOfCharge.CurrentEnergy vss:Vehicle.Chassis.SteeringWheel.Angle vss:Vehicle.CurrentLocation.Longitude vss:Vehicle.CurrentLocation.Latitude
```

#### Single signal to Information Layer Server via websocket
```
python3 rl-bridge.py --url $RL_BROKER_URL --x_api_key $RL_API_KEY -o information-layer --signals vss:Vehicle.Speed -i TEST_VEHICLE
```
#### Errors
If the requested signal or namespace name is not available in the broker an error will be reported:
```
python3 rl-bridge.py --url $RL_BROKER_URL --x_api_key $RL_API_KEY --signals bad-namespace-name:bad-signal-name
signal not declared (namespace, signal): ('bad-namespace-name', 'bad-signal-name')
```

`signal not declared` may also be returned as an error if you are using the RemotiveLabs Cloud Demo or Free Tier and you have not told the Broker in the cloud to `Prepare for playback`.

### Runtime example
The following example takes you through the steps of playing the RemotiveLabs dataset example `Night drive to Luftkastellet` from the RemotiveLabs Cloud free tier and using the bridge to record it into the playground.

#### Prepare the recording for playback in RemotiveLabs cloud:
1. Select `Night drive to Luftkastellet` from the list of recordings.
2. Select the `Playback` tab for that recording and then set `configuration_vss` as the `signal transformation`. This tells RemotiveLabs to send VSS data rather than the native vehicle data.
3. Select the `Prepare for playback` button.

#### Execute the bridge:

4. Execute the bridge for the signals you wish to subscribe too:
```
python3 rl-bridge.py --url $RL_BROKER_URL --x_api_key $RL_API_KEY -o iotdb --signals vss:Vehicle.Speed vss:Vehicle.Chassis.Accelerator.PedalPosition vss:Vehicle.Powertrain.Transmission.CurrentGear vss:Vehicle.Powertrain.TractionBattery.NominalVoltage vss:Vehicle.Powertrain.TractionBattery.StateOfCharge.CurrentEnergy vss:Vehicle.Chassis.SteeringWheel.Angle vss:Vehicle.CurrentLocation.Longitude vss:Vehicle.CurrentLocation.Latitude
```
5. The bridge will report that it is waiting to receive signals:
```
Broker connection and subscription setup completed, waiting for signals...
```

#### Start playback of the recording in RemotiveLabs cloud:

6. Press the play icon in the `Play` tab.
7. The bridge will print on the command line the data frames it is receiving in JSON format, e.g.:
```
Broker connection and subscription setup completed, waiting for signals...
{"timestamp_us": 1699472071567347, "name": "Vehicle.Chassis.SteeringWheel.Angle", "namespace": "vss", "value": -2.7000000000000455}
{"timestamp_us": 1699472071551128, "name": "Vehicle.CurrentLocation.Latitude", "namespace": "vss", "value": 55.569897999999995}
{"timestamp_us": 1699472071551128, "name": "Vehicle.CurrentLocation.Longitude", "namespace": "vss", "value": 12.897946}
{"timestamp_us": 1699472071578026, "name": "Vehicle.Chassis.SteeringWheel.Angle", "namespace": "vss", "value": -2.6000000000000227}
```
#### Stop the bridge:
8. To exit the bridge, e.g. when playback has finished, press the keyboard interrupt (Ctrl-C).

#### Known Errors
Startup of the bridge fails sometimes (when sending signals to Information Layer) if the playback is already going on. Solution is to pause the playback, start the bridge and then to continue the playback. 

#### Tips
Tip: Playback can be paused and restarted at any time.

Tip: You can check the data that has been stored using the IoTDB CLI client.

Example 1: Count the number of data items:
```
select count(*) from root.test2.dev1
+--------------------------------------+--------------------------------------------------------------------------+----------------------------------------------------------+---------------------------------------------------------+--------------------------------------------------------------------+------------------------------------------------------------------+-------------------------------------------------------------------+---------------------------------------------------------------------------------------+------------------------------------------------------------+
|count(root.test2.dev1.`Vehicle.Speed`)|count(root.test2.dev1.`Vehicle.Powertrain.TractionBattery.NominalVoltage`)|count(root.test2.dev1.`Vehicle.CurrentLocation.Longitude`)|count(root.test2.dev1.`Vehicle.CurrentLocation.Latitude`)|count(root.test2.dev1.`Vehicle.Powertrain.Transmission.CurrentGear`)|count(root.test2.dev1.`Vehicle.Chassis.Accelerator.PedalPosition`)|count(root.test2.dev1.`Vehicle.CurrentLocation.HorizontalAccuracy`)|count(root.test2.dev1.`Vehicle.Powertrain.TractionBattery.StateOfCharge.CurrentEnergy`)|count(root.test2.dev1.`Vehicle.Chassis.SteeringWheel.Angle`)|
+--------------------------------------+--------------------------------------------------------------------------+----------------------------------------------------------+---------------------------------------------------------+--------------------------------------------------------------------+------------------------------------------------------------------+-------------------------------------------------------------------+---------------------------------------------------------------------------------------+------------------------------------------------------------+
|                                 13519|                                                                       419|                                                       279|                                                      279|                                                               27033|                                                             27033|                                                                  0|                                                                                    419|                                                       27915|
+--------------------------------------+--------------------------------------------------------------------------+----------------------------------------------------------+---------------------------------------------------------+--------------------------------------------------------------------+------------------------------------------------------------------+-------------------------------------------------------------------+---------------------------------------------------------------------------------------+------------------------------------------------------------+
Total line number = 1
```

Example 2: Display `Vehicle.Speed` data:
```
select `Vehicle.Speed` from root.test2.dev1
+------------------------+-------------------------------+
|                    Time|root.test2.dev1.`Vehicle.Speed`|
+------------------------+-------------------------------+
|2023-11-08T19:34:39.651Z|                            0.0|
|2023-11-08T19:34:39.663Z|                            0.0|
|2023-11-08T19:34:39.679Z|                            0.0|
|2023-11-08T19:34:39.699Z|                            0.0|
|2023-11-08T19:34:39.718Z|                            0.0|
|2023-11-08T19:34:39.740Z|                            0.0|
...
```

## Developer notes
### Apache IoTDB
The code is simpler if we do not need to query IoTDB to see what timeseries schemas are in use. IoTDB can infer the data type from string data for defined schemas. So, we use this ability to achieve the simplicity goal and make the sample more readable.

We convert the incoming data from RemotiveLabs to string before writing to IoTDB, which in turn converts to the data type in the schema on ingest. Of course, if maximum performance was needed then such conversions would be avoided.

### RemotiveLabs samples
This bridge is based on the [`python subscribe`](https://github.com/remotivelabs/remotivelabs-samples/tree/1939efca4ce5e10a7001595bb4cbe81de4a9a74e/python/subscribe) sample from the RemotiveLabs [samples repository](https://github.com/remotivelabs/remotivelabs-samples).

Investigate the other samples for other ways in which you could interface the RemotiveLabs platform with the playground.
