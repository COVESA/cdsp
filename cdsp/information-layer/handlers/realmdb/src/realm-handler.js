const Realm = require('realm');
const config = require('../config/config');
const vehicleConfig = require('../config/vehicle-config'); // Import vehicle config file
const { v4: uuidv4 } = require('uuid'); // Importing UUID generator

const MediaElementSchema = {
  primaryKey: '_id',
  name: 'Vehicles',
  properties: {
    _id: 'int',
    Vehicle_Cabin_HVAC_AmbientAirTemperature: 'double'
  }
};

const app = new Realm.App({ id: config.realmAppId });
const credentials = Realm.Credentials.apiKey(config.realmApiKey);

const realmConfig = {
  schema: [MediaElementSchema],
  path: 'myrealm12.realm',
  sync: {
    user: null, // will be assigned after authentication
    flexible: true,
    error: error => {
      console.error('Realm sync error:', error);
    }
  }
};

const authenticateAndConnectToRealm = async (sendMessageToClients, onMediaElementChange) => {
  try {
    const user = await app.logIn(credentials);
    console.log('Successfully authenticated with Realm');

    realmConfig.sync.user = user;
    const realm = await Realm.open(realmConfig);
    console.log('Realm connection established successfully');

    const MediaElements = realm.objects('Vehicles').subscribe();
    console.log(MediaElements);

    const objectId = vehicleConfig.Vin; // Retrieve objectId from config file
    const mediaElement = realm.objectForPrimaryKey('Vehicles', objectId);
    console.log(mediaElement);

    // Set WebSocket server ID as the same objectId
    const websocketId = String(objectId);

    mediaElement.addListener((mediaElement, changes) => onMediaElementChange(mediaElement, changes, sendMessageToClients, websocketId));
  } catch (error) {
    console.error('Failed to authenticate with Realm:', error);
  }
};

const onMediaElementChange = (mediaElement, changes, sendMessageToClients, websocketId) => {
  if (changes.deleted) {
    console.log(`MediaElement is deleted: ${changes.deleted}`);
  } else {
    changes.changedProperties.forEach(prop => {
      console.log(`* the value of "${prop}" changed to ${mediaElement[prop]}`);
      
      // Generate a meaningful UUID for WebSocket response
      const uuid = uuidv4();

      const message = {
        type: 'update',
        tree: 'VSS',
        id: websocketId, // Use the WebSocket server ID
        uuid: uuid, // Use generated UUID
        dateTime: new Date().toISOString(),
        node: {
          name: prop, // Sending the property name as node name
          value: mediaElement[prop] // Sending the property value as node value
        }
      };
      sendMessageToClients(message);
    });
  }
};

module.exports = {
  authenticateAndConnectToRealm,
  onMediaElementChange
};
