import Ajv, {ValidateFunction, Schema} from "ajv";
import addFormats from "ajv-formats";
import addErrors from "ajv-errors";
import {NewMessageType, NewMessage} from "./NewMessage";
import {replaceDotsWithUnderscore} from "../../handlers/utils/transformations";

export interface ValidationResult {
  valid: boolean;
  errors?: string[];
  message?: NewMessage;
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


    // Add schemas for each request type
    this.schemas = {
      [NewMessageType.Get]: this.compileSchema({
        type: "object",
        required: ["type", "instance", "schema"],
        additionalProperties: false,
        properties: {
          type: {type: "string", const: "get"},
          path: {type: "string"},
          instance: {...notEmptyStringRule},
          schema: {...notEmptyStringRule},
          requestId: {type: "string"},
          format: {type: "string", enum: ["nested", "flat"]},
          root: {type: "string", enum: ["absolute", "relative"]},
        },
        errorMessage: {
          required: {
            type: "The 'type' field is required",
            path: "The 'path' field is required",
            instance: "The 'instance' field is required",
          },
          additionalProperties: "Unexpected property found",
          properties: {
            ...notEmptyStringErrorMessage("schema"),
            ...notEmptyStringErrorMessage("instance")
          }
        },
      }),
      [NewMessageType.Set]: this.compileSchema({
        type: "object",
        required: ["type", "schema", "instance", "data"],
        additionalProperties: false,
        properties: {
          type: {type: "string", const: "set"},
          path: {type: "string"},
          instance: {...notEmptyStringRule},
          schema: {...notEmptyStringRule},
          data: {},
          requestId: {type: "string"},
          metadata: {type: "object"},
          sync: {type: "string", enum: ["off", "local", "remote"]},
          timeseries: {type: "boolean"},
        },
        errorMessage: {
          required: {
            type: "The 'type' field is required",
            path: "The 'path' field is required",
            instance: "The 'instance' field is required",
            data: "The 'data' field is required",
          },
          additionalProperties: "Unexpected property found",
          properties: {
            ...notEmptyStringErrorMessage("schema"),
            ...notEmptyStringErrorMessage("instance")
          }
        },
      }),
      [NewMessageType.Subscribe]: this.compileSchema({
        type: "object",
        required: ["type", "instance", "schema"],
        additionalProperties: false,
        properties: {
          type: {type: "string", enum: ["subscribe"]},
          path: {type: "string"},
          instance: {...notEmptyStringRule},
          schema: {...notEmptyStringRule},
          requestId: {type: "string"},
          format: {type: "string", enum: ["nested", "flat"]},
          root: {type: "string", enum: ["absolute", "relative"]},
        },
        errorMessage: {
          required: {
            type: "The 'type' field is required",
            instance: "The 'instance' field is required",
            schema: "The 'schema' field is required",
          },
          additionalProperties: "Unexpected property found",
          properties: {
            ...notEmptyStringErrorMessage("schema"),
            ...notEmptyStringErrorMessage("instance")
          }
        },
      }),
      [NewMessageType.Unsubscribe]: this.compileSchema({
        type: "object",
        required: ["type", "instance", "schema"],
        additionalProperties: false,
        properties: {
          type: {type: "string", enum: ["unsubscribe"]},
          path: {type: "string"},
          instance: {...notEmptyStringRule},
          schema: {...notEmptyStringRule},
          requestId: {type: "string"},
        },
        errorMessage: {
          required: {
            type: "The 'type' field is required",
            instance: "The 'instance' field is required",
            schema: "The 'schema' field is required",
          },
          additionalProperties: "Unexpected property found",
          properties: {
            ...notEmptyStringErrorMessage("schema"),
            ...notEmptyStringErrorMessage("instance")
          }
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
    const schemaKey = this.typeToSchemaKey[request.type];
    if (!schemaKey) {
      return {valid: false, errors: [`Unknown request type: ${request.type}`]};
    }

    const validator = this.schemas[schemaKey];

    if (!validator) {
      return {valid: false, errors: [`Unknown request type: ${request.type}`]};
    }

    // Validate the parsed object
    const valid = validator(request);

    if (valid) {
      this.modifyRequest(request);
      return {valid: true, message: request as NewMessage}; // Cast to NewMessage
    } else {
      return {
        valid: false,
        errors: validator.errors?.map((error) => error.message || "") || [],
      };
    }
  }

  /**
   * Apply modifications to the request structure and transformations to request values.
   *
   * @param request - The request object to modify.
   */
  private modifyRequest(request: any) {
    this.moveSchemaToPathField(request);
    request.path = replaceDotsWithUnderscore(request.path);
  }

  /**
   * Merges `schema` into `path` and removes `schema`.
   * If `path` is missing, sets it to `schema`.
   *
   * @param request - The request object to modify.
   */
  private moveSchemaToPathField(request: any) {
    if (request.schema && request.path) {
      request.path = `${request.schema}_${request.path}`;
    } else if (request.schema && !request.path) {
      request.path = `${request.schema}`;
    }
    delete request.schema;
  }
}

// Generic validation rule: Ensure a string is not empty or blank
const notEmptyStringRule = {
  type: "string",
  minLength: 1,  // Prevents empty strings ("" )
  pattern: "^(?!\\s*$).+" // Ensures the string is not just whitespace ("  ")
};

// Generic error message: Used for any field requiring a non-empty string
const notEmptyStringErrorMessage = (fieldName: string) => ({
  [fieldName]: `The '${fieldName}' field must contain at least one non-whitespace character`
});
