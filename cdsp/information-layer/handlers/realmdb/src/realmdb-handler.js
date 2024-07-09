const Realm = require("realm");
const Handler = require('../../handler')
const config = require("../config/config");
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

const sendMessageToClient = (ws, message) => {
  ws.send(JSON.stringify(message));
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

      const MediaElements = this.realm.objects("Vehicles").subscribe(); // TODO: Is it necessary to log this, extra function?
      if (MediaElements) {
        console.log(MediaElements); 
      } else {
        this.sendMessageToClients({ error: "Vehicles collection not found for subscription"});
      }
    } catch (error) {
      console.error("Failed to authenticate with Realm:", error);
    }
  }

  async read(message, ws) {
    try{
      const objectId = message.data.Vin; // Retrieve objectId from config file
      const MediaElement = this.realm.objectForPrimaryKey("Vehicles", objectId);
      if (MediaElement) {
        const response = {
          type: "read_response",
          data: MediaElement,
        };
        sendMessageToClient(ws, JSON.stringify(response));
      } else {
        sendMessageToClient(ws, JSON.stringify({ error: 'Object not found' }));
      }
    } catch (error) {
      console.error("Error reading object from Realm:", error);
      sendMessageToClient(ws, JSON.stringify({ error: 'Error reading object' }));
    }
  }

  async subscribe(message, ws) {    
    try {
      const objectId = message.data.Vin;
      console.log(`Subscribing element: ${objectId}`)
      const MediaElement = await this.realm.objectForPrimaryKey("Vehicles", objectId);

      if (MediaElement) {
        const websocketId = String(objectId);
        MediaElement.addListener((MediaElement, changes) =>
          this.onMediaElementChange(MediaElement, changes, websocketId)
        );
        sendMessageToClient(ws, { success: `Subscribed to changes for object ID ${objectId}` })
      } else {
        sendMessageToClient(ws, JSON.stringify({ error: 'Object not found' }));
      }
    } catch (error) {
      console.error("Error subscribing to object changes in Realm:", error);
      sendMessageToClient(ws, { error: 'Error subscribing to object changes' });
    }
  }

  onMediaElementChange(MediaElement, changes, websocketId) {
    if (changes.deleted) {
      console.log(`MediaElement is deleted: ${changes.deleted}`);
    } else {
      changes.changedProperties.forEach((prop) => {
        console.log(`* the value of "${prop}" changed to ${MediaElement[prop]}`);

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
          value: MediaElement[prop], // Sending the property value as node value
          },
        };
        this.sendMessageToClients(message);
      });
    }
  }
}

module.exports = RealmDBHandler;