# RealmDB

This directory contains the RealmDB Handler as Node.js application. As [RealmDB](https://www.mongodb.com/docs/atlas/device-sdks/sdk/node/) is an embedded database, the RealmDB Handler directly embeds the RealmSDK which creates the [RealmDB database](https://github.com/realm/realm-js) file(s) automatically in the working directory during runtime of RealmDB Handler.

# Features

- **Authentication**: Authenticates with MongoDB Realm using an API key.
- **Get Data**: Retrieves data from the Realm database using a VIN (for VSS object) object ID.
- **Set Data**: Write data to the Realm database using a VIN (for VSS object) as object ID.- 
- **Subscribe to Changes**: Listens to changes in specific data objects using VIN (for VSS object) as object ID and sends updates to WebSocket clients.
- **Unsubscribe to Changes**: Unsubscribe listener to a specific data object using VIN (for VSS object) as object ID.
- **Unsubscribe client**: Unsubscribe all listeners applied to a client.
- **Error Handling**: Logs and handles errors during database operations and synchronization.

# Configure RealmDB

Before the Database-Router can start the RealmDB Handler without any errors you need to start and run Docker containers defined in a [Docker Compose file](/docker/).

## Create a ATLAS Cloud instance

To get APIKey and AppID you need to set up a [ATLAS cloud](https://cloud.mongodb.com/) instance and App Services. There is a free Tier solution (Status as of May 29, 2024) and you will find a lot of documentation in the internet how to set up everything.

## Configure of a RealmDB Handler

Create (if it does not exist) `/docker/.env` and add the following environment variables, replacing the app id and the api key with yours.

```sh
    #########################
    # GENERAL CONFIGURATION #
    #########################
    
    # HANDLER_TYPE define the database to initialize
    HANDLER_TYPE=realmdb
    # DATA_POINTS_SCHEMA_FILE is the YAML or JSON file containing all data points supported. See the ../../config/README.md for more information.
    DATA_POINTS_SCHEMA_FILE=vss_data_points.yaml
    #########################
    # REALMDB CONFIGURATION #
    #########################
    
    # VERSION must be 0 or a positive integer. This is used for versioning the RealmDB configuration schema.
    VERSION_REALMDB_SCHEMA=0
    
    # Access to ATLAS Cloud instance
    REALMDB_APP_ID="your-app-key"
    REALMDB_API_KEY="your-api-key"
```

> [!WARNING] 
> Do not commit this file to GitHub!

## Starting the RealmDB handler

You do not need to start RealmDB Handler manually. It is started by the DB-Router like described [here](../../../README.md).
