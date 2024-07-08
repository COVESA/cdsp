const Realm = require("realm");
const Handler = require('../../handler')
const config = require("../config/config");
//const vehicleConfig = require("../config/vehicle-config"); // Import vehicle config file
const { v4: uuidv4 } = require("uuid"); // Importing UUID generator

const MediaElementSchema = {
  primaryKey: "_id",
  name: "Vehicles",
  properties: {
    _id: "int",
    Vehicle_Cabin_HVAC_AmbientAirTemperature: "double",
  },
};

const app = new Realm.App({ id: config.realmAppId });
const credentials = Realm.Credentials.apiKey(config.realmApiKey);

const realmConfig = {
  schema: [MediaElementSchema],
  path: "myrealm12.realm",
  sync: {
    user: null, // will be assigned after authentication
    flexible: true,
    error: (error) => {
      console.error("Realm sync error:", error);
    },
  },
};

class RealmDBHandler extends Handler {
  constructor() {
    super();
    this.realm = null;
    this.sendMessageToClients = null;
  }

  async authenticateAndConnect(sendMessageToClients) {
    try {
      this.sendMessageToClients = sendMessageToClients;
      const user = await app.logIn(credentials);
      console.log("Successfully authenticated with Realm");

      realmConfig.sync.user = user;
      this.realm = await Realm.open(realmConfig);
      console.log("Realm connection established successfully");

      const MediaElements = this.realm.objects("Vehicles").subscribe();
      console.log(MediaElements); // TODO: Is it necessary to log this?
    } catch (error) {
      console.error("Failed to authenticate with Realm:", error);
    }
  }

  async read(message, ws) {
    try{
      const objectId = message.data.Vin; // Retrieve objectId from config file
      const mediaElement = this.realm.objectForPrimaryKey("Vehicles", objectId);
      console.log(mediaElement);
      if (mediaElement) {
        const response = {
          type: "read_response",
          data: mediaElement,
        };
        ws.send(JSON.stringify(response));
      } else {
        ws.send(JSON.stringify({ error: 'Object not found' }));
      }
    } catch (error) {
      console.error("Error reading object from Realm:", error);
      ws.send(JSON.stringify({ error: 'Error reading object' }));
    }
  }

  async write(message, ws) {
    // Implement write logic for RealmDB
  }

  async subscribe(message, ws) {
    const sendMessageToClient = (message) => {
      ws.send(JSON.stringify(message));
    };

    try {
      const objectId = message.data.Vin;
      console.log(`Subscribing element: ${objectId}`)
      const mediaElement = await this.realm.objectForPrimaryKey("Vehicles", objectId);
      console.log(mediaElement);

      if (mediaElement) {
        const websocketId = String(objectId);
        mediaElement.addListener((mediaElement, changes) =>
          this.onMediaElementChange(mediaElement, changes, websocketId)
        );
        sendMessageToClient({ success: `Subscribed to changes for object ID ${objectId}` })
      } else {
        sendMessageToClient({ error: "Vehicles collection not found for subscription"});
      }
    } catch (error) {
      console.error("Error subscribing to object changes in Realm:", error);
      sendMessageToClient({ error: 'Error subscribing to object changes' });
    }
  }

  onMediaElementChange(mediaElement, changes, websocketId) {
    if (changes.deleted) {
      console.log(`MediaElement is deleted: ${changes.deleted}`);
    } else {
      changes.changedProperties.forEach((prop) => {
        console.log(`* the value of "${prop}" changed to ${mediaElement[prop]}`);

        // Generate a meaningful UUID for WebSocket response
        const uuid = uuidv4();
        
        const message = {
          type: "update",
          tree: "VSS",
          id: websocketId, // Use the WebSocket server ID
          uuid: uuid, // Use generated UUID
          dateTime: new Date().toISOString(),
          node: {
            name: prop, // Sending the property name as node name
          value: mediaElement[prop], // Sending the property value as node value
          },
        };
        this.sendMessageToClients(message);
      });
    }
  }
}

module.exports = RealmDBHandler;