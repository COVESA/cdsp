This directory contains the RealmDB Handler as Node.js application. As [RealmDB](https://www.mongodb.com/docs/atlas/device-sdks/sdk/node/) is an embedded database, the RealmDB Handler directly embedds the RealmSDK which creates the [RealmDB database](https://github.com/realm/realm-js) file(s) automatically in the working directory during runtime of RealmDB Handler.

## 1 Installation of RealmDB-Handler

Execute within <code>realmdb/src</code> directory

     npm install

## 2 Configuring RealmDB
Before the Database-Router can start the RealmDB Handler without any errors you need to configure the RealmSDK before. 

### 2.1 Create ATLAS Cloud instance
To get APIKey and AppID you need to setup a [ATLAS cloud](https://cloud.mongodb.com/) instance and App Services. There is a free Tier solution (Status as of May 29, 2024) and you will find a lot of documentation in the internet how to set up everything.

### 2.2 Configuration of RealmDB Handler

Create a file <code>config.js</code> in the existing <code>realmdb/config</code> directory and add this content:

<code>
 module.exports = { <br>
     realmAppId: '<i>your-realm-AppId</i>', <br>
     realmApiKey: '<i>your-realm-ApiKey</i>'<br>
  };
</code>

> **_IMPORTANT:_**  Do not commit this file to github!


### 2.3 Configuration of example vehicle
In the already existing file <code>vehicle-config.js</code> you can change the VIN (Vehicle Identification Number) of the example vehicle. The default VIN is <i>1234567</i>. If you do not want to change it ensure, that in your ATLAS cloud instance there is a vehicle <i>document</i> with an <code>_id: <i>1234567</i></code> in a collection named <i>Vehicles</i>. More infos how to run an example together with ATLAS cloud you can find [here](../readme.md#case-1-you-choosed-realm-as-database).

## 3 Starting RealmDB-Handler
You do not need to start RealmDB Handler manually. It is started by the DB-Router like described [here](../readme.md#22-installation-of-database-router).

