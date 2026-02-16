import Ajv, {ValidateFunction, Schema} from "ajv";
import addFormats from "ajv-formats";
import addErrors from "ajv-errors";
import {NewMessageType} from "./NewMessage";
import {NewMessageDTO} from "./MessageDTO";

export interface ValidationResult {
  valid: boolean;
  errors?: string[];
  messageDTO?: NewMessageDTO;
}

export class RequestValidator {
  private ajv: Ajv;
  private schemas: Record<NewMessageType, ValidateFunction>;

  constructor() {
    this.ajv = new Ajv({
      allErrors: true,
      strict: false,
    });
    addFormats(this.ajv);
    addErrors(this.ajv);

    // Base JSON-RPC envelope scheme
    const baseRpcSchema = {
      type: "object",
      required: ["jsonrpc", "method", "id", "params"],
      additionalProperties: false,
      properties: {
        jsonrpc: {type: "string", const: "2.0"},
        method: {type: "string"}, // overridden in each message
        id: {...stringOrIntegerRule},
        params: {type: "object"}, // overridden in each message
      },
      errorMessage: {
        required: {
          jsonrpc: "The 'jsonrpc' field (must be '2.0') is required",
          method: "The 'method' field is required",
          id: "The 'id' field is required",
          params: "The 'params' field is required",
        },
        additionalProperties: "Unexpected property found at top level",
        properties: {
          jsonrpc: "The 'jsonrpc' version must be the string '2.0'",
          ...stringOrIntegerErrorMessage("id")
        }
      },
    };

    // Base params schema for all messages
    const baseParamsSchema = {
      type: "object",
      required: ["instance", "schema"],
      additionalProperties: false,
      properties: {
        path: {type: "string"},
        instance: {...stringValidation},
        schema: {...stringValidation},
      },
      errorMessage: {
        required: {
          instance: "The 'instance' field is required",
          schema: "The 'schema' field is required",
        },
        additionalProperties: "Unexpected property found in params",
        properties: {
          ...notEmptyStringErrorMessage("instance"),
          ...notEmptyStringErrorMessage("schema"),
        },
      },
    };
    // Add schemas for each request type
    this.schemas = {
      [NewMessageType.Get]: this.compileSchema({
        ...baseRpcSchema,
        properties: {
          ...baseRpcSchema.properties,
          method: {type: "string", const: "get"},
          params: {
            ...baseParamsSchema,
            properties: {
              ...baseParamsSchema.properties,
              format: {type: "string", enum: ["nested", "flat"]},
              root: {type: "string", enum: ["absolute", "relative"]},
            },
          },
        },
      }),
      [NewMessageType.Set]: this.compileSchema({
        ...baseRpcSchema,
        properties: {
          ...baseRpcSchema.properties,
          method: {type: "string", const: "set"},
          params: {
            ...baseParamsSchema,
            required: [...baseParamsSchema.required, "data"],
            properties: {
              ...baseParamsSchema.properties,
              data: {},
              metadata: {type: "object"},
              sync: {type: "string", enum: ["off", "local", "remote"]},
              timeseries: {type: "boolean"},
            },
            errorMessage: {
              ...baseParamsSchema.errorMessage,
              required: {
                ...baseParamsSchema.errorMessage.required,
                data: "The 'data' field is required",
              },
            },
          },
        },
      }),
      [NewMessageType.Subscribe]: this.compileSchema({
        ...baseRpcSchema,
        properties: {
          ...baseRpcSchema.properties,
          method: {type: "string", const: "subscribe"},
          params: {
            ...baseParamsSchema,
            properties: {
              ...baseParamsSchema.properties,
              format: {type: "string", enum: ["nested", "flat"]},
              root: {type: "string", enum: ["absolute", "relative"]},
            },
          },
        },
      }),
      [NewMessageType.Unsubscribe]: this.compileSchema({
        ...baseRpcSchema,
        properties: {
          ...baseRpcSchema.properties,
          method: {type: "string", const: "unsubscribe"},
          params: {
            ...baseParamsSchema,
          },
        },
      }),
      [NewMessageType.PermissionsEdit]: this.compileSchema({
        type: "object",
        required: ["type", "userId"],
        additionalProperties: false,
        properties: {
          type: {type: "string", const: "permissions/edit"},
          userId: {type: "string"},
          allow: {
            type: "array",
            items: {type: "string"},
          },
          deny: {
            type: "array",
            items: {type: "string"},
          },
          delete: {
            type: "array",
            items: {type: "string"},
          },
          requestId: {type: "string"},
        },
        errorMessage: {
          required: {
            type: "The 'type' field is required",
            userId: "The 'userId' field is required",
          },
          additionalProperties: "Unexpected property found",
        },
      }),
      [NewMessageType.TimeseriesGet]: this.compileSchema({
        type: "object",
        required: ["type", "path", "instance"],
        additionalProperties: false,
        properties: {
          type: {type: "string", const: "timeseries/get"},
          path: {type: "string"},
          instance: {type: "string"},
          requestId: {type: "string", nullable: true},
          query: {
            type: "object",
            additionalProperties: false,
            properties: {
              gte: {
                type: "object",
                required: ["seconds", "nanos"],
                additionalProperties: false,
                properties: {
                  seconds: {type: "number"},
                  nanos: {type: "number"},
                },
              },
              lte: {
                type: "object",
                required: ["seconds", "nanos"],
                additionalProperties: false,
                properties: {
                  seconds: {type: "number"},
                  nanos: {type: "number"},
                },
              },
            },
            nullable: true,
          },
        },
        errorMessage: {
          required: {
            type: "The 'type' field is required",
            path: "The 'path' field is required",
            instance: "The 'instance' field is required",
          },
          additionalProperties: "Unexpected property found",
        },
      }),
    };

  }

  private typeToSchemaKey: Record<string, NewMessageType> = {
    "get": NewMessageType.Get,
    "set": NewMessageType.Set,
    "subscribe": NewMessageType.Subscribe,
    "unsubscribe": NewMessageType.Unsubscribe,
    "permissions/edit": NewMessageType.PermissionsEdit,
    "timeseries/get": NewMessageType.TimeseriesGet,
  };

  private compileSchema(schema: Schema): ValidateFunction {
    return this.ajv.compile(schema);
  }

  validate(jsonString: string): ValidationResult {
    let request: any;

    // Parse the JSON string into an object
    try {
      request = JSON.parse(jsonString);
    } catch (error) {
      return {valid: false, errors: ["Invalid JSON format"]};
    }

    // Use the mapping to find the schema key
    const schemaKey = this.typeToSchemaKey[request.method];
    if (!schemaKey) {
      return {valid: false, errors: [`Unknown request method: ${request.method}`], messageDTO: request as NewMessageDTO};
    }

    const validator = this.schemas[schemaKey];

    if (!validator) {
      return {valid: false, errors: [`Unknown request method: ${request.method}`], messageDTO: request as NewMessageDTO};
    }

    // Validate the parsed object
    const valid = validator(request);

    if (valid) {
      return {valid: true, messageDTO: request as NewMessageDTO};
    } else {
      return {
        valid: false,
        errors: validator.errors?.map((error) => error.message || "") || [],
        messageDTO: request as NewMessageDTO,
      };
    }
  }
}

// Generic validation rule: Ensure a string is not empty or blank
const stringValidation = {
  type: "string",
  minLength: 1,  // Prevents empty strings ("")
  pattern: "^(?!\\s*$).+" // Ensures the string is not just whitespace ("  ")
};

// Generic validation rule: Ensure a string is not empty/blank, OR accept integers
const stringOrIntegerRule = {
  anyOf: [
    stringValidation,
    {
      type: "integer",
    },
  ],
};

// Generic error message: Used for any field requiring a non-empty string or integer
const stringOrIntegerErrorMessage = (fieldName: string) => ({
  [fieldName]: `The '${fieldName}' field must be a non-empty string or an integer`,
});

// Generic error message: Used for any field requiring a non-empty string
const notEmptyStringErrorMessage = (fieldName: string) => ({
  [fieldName]: `The '${fieldName}' field must contain at least one non-whitespace character`
});
