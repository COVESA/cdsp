# KUKSA CAN Provider Feeder
The KUKSA project contains the component [KUKSA CAN Provider](https://github.com/eclipse-kuksa/kuksa-can-provider) which provides a CAN to VSS feeder with DBC support. Supported sources include CAN dumps and real and virtual SocketCAN connections. The Provider uses a translation table to translate CAN data into the VSS data model northbound.

The COVESA Central Data Service Playground (CDSP) project has extended the CAN Provider to support writing to the Apache IoTDB timeseries database used in CDSP as a new northbound client type.

The sources for the fork can be found [here](https://github.com/slawr/kuksa-can-provider.git) in the branch [covesa-cdsp](https://github.com/slawr/kuksa-can-provider/tree/covesa-cdsp).

Documentation for how to use it, including an example of playing back a CAN dump via a virtual SocketCAN, can be found in the file [README-covesa-cdsp-iotdb.md](https://github.com/slawr/kuksa-can-provider/blob/covesa-cdsp/README-covesa-cdsp-iotdb.md)

The intention is to talk to the KUKSA project about upstreaming the new client support.
