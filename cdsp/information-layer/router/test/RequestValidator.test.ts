import { RequestValidator } from "../utils/RequestValidator"; 

describe("RequestValidator", () => {
  let validator: RequestValidator;

  beforeEach(() => {
    validator = new RequestValidator();
  });

  const validRequests = [
    {
      name: "Valid get request",
      json: `{
        "jsonrpc": "2.0",
        "method": "get",
        "id": "123",
        "params": {
          "schema": "Vehicle",
          "instance": "SMT905JN26J262542"
        }
      }`,
      expectedPath: "Vehicle",
    },
    {
      name: "Valid set request",
      json: `{
        "jsonrpc": "2.0",
        "method": "set",
        "id": "123",
        "params": {
          "schema": "Vehicle",
          "path": "Speed",
          "instance": "SMT905JN26J262542",
          "data": 100
        }
      }`,
      expectedPath: "Vehicle_Speed",
    },
    {
      name: "Valid subscribe request",
      json: `{
        "jsonrpc": "2.0",
        "method": "subscribe",
        "id": "123",
        "params": {
          "schema": "Vehicle",
          "instance": "SMT905JN26J262542"
        }
      }`,
      expectedPath: "Vehicle",
    },
    {
      name: "Valid unsubscribe request",
      json: `{
        "jsonrpc": "2.0",
        "method": "unsubscribe",
        "id": "123",
        "params": {
          "schema": "Vehicle",
          "instance": "SMT905JN26J262542"
        }
      }`,
      expectedPath: "Vehicle",
    }  
  ];

  const invalidRequests = [
    {
      name: "Additional not specified property in get",
      json: `{
        "jsonrpc": "2.0",
        "method": "get",
        "id": "123",
        "params": {
          "path": "Speed",
          "schema": "Vehicle",
          "instance": "SMT905JN26J262542",
          "extraProperty": "should not be here"
        }
      }`,
      error: "Unexpected property found in params",
    },
    {
      name: "Missing required property in get",
      json: `{
        "jsonrpc": "2.0",
        "method": "get",
        "id": "123",
        "params": {
          "path": "Speed",
          "schema": "Vehicle"
        }
      }`,
      error: "The 'instance' field is required",
    },
    {
      name: "Schema is blank",
      json: `{
        "jsonrpc": "2.0",
        "method": "get",
        "id": "123",
        "params": {
          "path": "Speed",
          "schema": "",
          "instance": "SMT905JN26J262542"
        }
      }`,
      error: "The 'schema' field must contain at least one non-whitespace character",
    },
    {
      name: "Schema contains only whitespace characters",
      json: `{
        "jsonrpc": "2.0",
        "method": "get",
        "id": "123",
        "params": {
          "path": "Speed",
          "schema": "   ",
          "instance": "SMT905JN26J262542"
        }
      }`,
      error: "The 'schema' field must contain at least one non-whitespace character",
    },
    {
      name: "Instance is blank",
      json: `{
        "jsonrpc": "2.0",
        "method": "get",
        "id": "123",
        "params": {
          "path": "Speed",
          "instance": "",
          "schema": "SMT905JN26J262542"
        }
      }`,
      error: "The 'instance' field must contain at least one non-whitespace character",
    },
    {
      name: "Instance contains only whitespace characters",
      json: `{
        "jsonrpc": "2.0",
        "method": "get",
        "id": "123",
        "params": {
          "path": "Speed",
          "instance": "   ",
          "schema": "Vehicle"
        }
      }`,
      error: "The 'instance' field must contain at least one non-whitespace character",
    },
    {
      name: "Unknown request 'method'",
      json: `{
        "jsonrpc": "2.0",
        "method": "unknownType",
        "id": "123",
        "params": {
          "path": "Speed",
          "schema": "Vehicle",
          "instance": "Vehicle"
        }
      }`,
      error: "Unknown request method: unknownType",
    }
  ];

  describe("Valid Requests", () => {
    validRequests.forEach(({ name, json }) => {
      test(name, () => {
        const result = validator.validate(json);
        expect(result.valid).toBe(true);

      });
    });
  });

  describe("Invalid Requests", () => {
    invalidRequests.forEach(({ name, json, error }) => {
      test(name, () => {
        const result = validator.validate(json);
  
        // Ensure the result is invalid before accessing errors
        if (!result.valid) {
          expect(result.errors).toEqual(expect.arrayContaining([error]));
        } else {
          // Fail the test if the result is valid when it shouldn't be
          throw new Error(`Expected validation to fail, but it passed. Result: ${JSON.stringify(result)}`);
        }
      });
    });
  });
  
});
