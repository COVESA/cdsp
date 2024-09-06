const Ajv = require("ajv");
const ajv = new Ajv({
  allErrors: true,
  strict: true,
  allowUnionTypes: true,
});

const itemsHeader = ["type", "tree", "id", "uuid"];

const createCommonStructure = (requestType) => ({
  type: { type: "string", enum: [requestType] },
  tree: { type: "string", enum: ["VSS"] },
  id: { type: "string" },
  uuid: { type: "string" },
});

const schemas = {
  read: {
    type: "object",
    properties: {
      ...createCommonStructure("read"),
      node: {
        type: "object",
        properties: {
          name: { type: "string" },
        },
        required: ["name"],
        additionalProperties: false,
      },
    },
    required: itemsHeader.concat("node"),
    additionalProperties: false,
  },
  multiNodeRead: {
    type: "object",
    properties: {
      ...createCommonStructure("read"),
      nodes: {
        type: "array",
        items: {
          type: "object",
          properties: {
            name: { type: "string" },
          },
          required: ["name"],
          additionalProperties: false,
        },
      },
    },
    required: itemsHeader.concat("nodes"),
    additionalProperties: false,
  },
  write: {
    type: "object",
    properties: {
      ...createCommonStructure("write"),
      node: {
        type: "object",
        properties: {
          name: { type: "string" },
          value: { type: ["number", "string", "boolean"] },
        },
        required: ["name", "value"],
        additionalProperties: false,
      },
    },
    required: itemsHeader.concat("node"),
    additionalProperties: false,
  },
  multiNodeWrite: {
    type: "object",
    properties: {
      ...createCommonStructure("write"),
      nodes: {
        type: "array",
        items: {
          type: "object",
          properties: {
            name: { type: "string" },
            value: { type: ["number", "string", "boolean"] },
          },
          required: ["name", "value"],
          additionalProperties: false,
        },
      },
    },
    required: itemsHeader.concat("nodes"),
    additionalProperties: false,
  },
  subscribe: {
    type: "object",
    properties: {
      ...createCommonStructure("subscribe"),
    },
    required: itemsHeader,
    additionalProperties: false,
  },
  unsubscribe: {
    type: "object",
    properties: {
      ...createCommonStructure("unsubscribe"),
    },
    required: itemsHeader,
    additionalProperties: false,
  },
};

const validateMessage = (message) => {
  try {
    const parsedMessage = JSON.parse(message);
    let schemaKey;
    switch (parsedMessage.type) {
      case "read":
        schemaKey = parsedMessage.nodes ? "multiNodeRead" : "read";
        break;
      case "write":
        schemaKey = parsedMessage.nodes ? "multiNodeWrite" : "write";
        break;
      case "subscribe":
        schemaKey = "subscribe";
        break;
      case "unsubscribe":
        schemaKey = "unsubscribe";
        break;
      default:
        throw new Error(ajv.errorsText(validate.errors));
    }

    const schema = schemas[schemaKey];
    const validate = ajv.compile(schema);
    const valid = validate(parsedMessage);

    if (valid) {
      return parsedMessage;
    } else {
      throw new Error(ajv.errorsText(validate.errors));
    }
  } catch (error) {
    return new Error(`Invalid JSON message: ${error.message}`);
  }
};

module.exports = {
  validateMessage,
};
