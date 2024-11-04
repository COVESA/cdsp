import Ajv, { ErrorObject } from "ajv";
import { createErrorMessage } from "../../utils/error-message-helper";
import {
  ErrorMessage,
  Message,
  STATUS_ERRORS,
} from "../../handlers/utils/data-types";

// Initialize AJV with specific options
const ajv = new Ajv({
  allErrors: true,
  strict: true,
  allowUnionTypes: true,
});

// Define the headers for validation
const itemsHeader = ["type", "tree", "id", "uuid"];
const ERROR_CATEGORY = "messageValidation";

// Standard error message template
const standardError: ErrorMessage = {
  category: ERROR_CATEGORY,
  statusCode: STATUS_ERRORS.BAD_REQUEST,
  message: "Received an invalid message.",
};

// Helper to create the common structure for different message types
const createCommonStructure = (requestType: string) => ({
  type: { type: "string", enum: [requestType] },
  tree: { type: "string", enum: ["VSS"] },
  id: { type: "string" },
  uuid: { type: "string" },
});

// Define the schemas for message validation
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

// Function to customize error messages from AJV validation
const customizeErrorMessage = (
  errors: ErrorObject[] | null | undefined,
): ErrorMessage[] => {
  if (!errors) return [standardError];

  return errors.map((err) => {
    const { keyword, instancePath, params } = err;

    switch (keyword) {
      case "required":
        return createErrorMessage(
          ERROR_CATEGORY,
          STATUS_ERRORS.BAD_REQUEST,
          `Missing required field: ${params.missingProperty}.`,
        );
      case "type":
        return createErrorMessage(
          ERROR_CATEGORY,
          STATUS_ERRORS.BAD_REQUEST,
          `Invalid type for field '${instancePath}': expected ${params.type}.`,
        );
      case "enum":
        if (err.instancePath === "/tree") {
          return createErrorMessage(
            ERROR_CATEGORY,
            STATUS_ERRORS.NOT_FOUND,
            `Unsupported message 'tree': must be one of ${params.allowedValues?.join(", ")}`,
          );
        }
        break;
      case "additionalProperties":
        return createErrorMessage(
          ERROR_CATEGORY,
          STATUS_ERRORS.NOT_FOUND,
          `Unsupported property '${params.additionalProperty}' found`,
        );
      default:
        return createErrorMessage(
          ERROR_CATEGORY,
          STATUS_ERRORS.BAD_REQUEST,
          `Validation error on field '${instancePath}': ${err.message}.`,
        );
    }

    return standardError;
  });
};

/**
 * Validates a JSON message against predefined schemas.
 *
 * @param message - The JSON message to be validated.
 * @returns The parsed message if valid, otherwise throws an error.
 */
export const validateMessage = (message: string): Message | Error => {
  try {
    // Try to parse the JSON message
    const parsedMessage: Message = JSON.parse(message);

    // Determine the schema key based on the message type
    let schemaKey: keyof typeof schemas;
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
        const error = JSON.stringify([
          createErrorMessage(
            "messageValidation",
            STATUS_ERRORS.NOT_FOUND,
            `Unsupported message type (${parsedMessage.type})`,
          ),
        ]);
        throw new Error(error);
    }

    // Validate the parsed message against the schema
    const validate = ajv.compile(schemas[schemaKey]);

    if (!validate(parsedMessage)) {
      const customError = customizeErrorMessage(validate.errors);
      throw new Error(JSON.stringify(customError));
    }
    return parsedMessage;
  } catch (error: unknown) {
    if (error instanceof SyntaxError) {
      // Handle JSON parsing error
      const jsonError = JSON.stringify([
        createErrorMessage(
          "messageValidation",
          STATUS_ERRORS.NOT_FOUND,
          "The JSON format in the message is not valid.",
        ),
      ]);
      return new Error(jsonError);
    } else {
      // Handle other errors
      const errMsg = error instanceof Error ? error.message : "Unknown error";
      return new Error(errMsg);
    }
  }
};
