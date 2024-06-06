This directory contains the RealmDB Handler as Node.js application. As [RealmDB](https://www.mongodb.com/docs/atlas/device-sdks/sdk/node/) is an embedded database, the RealmDB Handler directly embedds the RealmSDK which creates the [RealmDB database](https://github.com/realm/realm-js) file(s) automatically in the working directory during runtime of RealmDB Handler.

# Installation

Execute within `realmdb` directory

```bash
npm install
```

# Configure RealmDB

Before the Database-Router can start the RealmDB Handler without any errors you need to configure the RealmSDK before.

## Create a ATLAS Cloud instance

To get APIKey and AppID you need to setup a [ATLAS cloud](https://cloud.mongodb.com/) instance and App Services. There is a free Tier solution (Status as of May 29, 2024) and you will find a lot of documentation in the internet how to set up everything.

## Configure of a RealmDB Handler

Create `config/config.js` with the following format, replacing the app id and the api key with yours.

```js
module.exports = {
  realmAppId: "your-realm-AppId",
  realmApiKey: "your-realm-ApiKey",
};
```

> **_IMPORTANT:_** Do not commit this file to github!

## Configure an example vehicle

Change the VIN (Vehicle Identification Number) of the example vehicle in [vehicle-config](./config/vehicle-config.js).
The default VIN is `1234567`.
If you do not want to change it ensure, that in your ATLAS cloud instance there is a vehicle _document_ with an `_id: 1234567` in a collection named `Vehicles`.
More infos how to run an example together with ATLAS cloud you can find [here](../readme.md#case-1-you-choosed-realm-as-database).

## Starting the RealmDB handler

You do not need to start RealmDB Handler manually. It is started by the DB-Router like described [here](../readme.md#22-installation-of-database-router).
