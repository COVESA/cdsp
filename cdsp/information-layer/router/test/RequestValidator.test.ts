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
        "type": "get",
        "schema": "Vehicle",
        "instance": "SMT905JN26J262542"
      }`,
      expectedPath: "Vehicle",
    },
    {
      name: "Valid set request",
      json: `{
        "type": "set",
        "schema": "Vehicle",
        "path": "Speed",
        "instance": "SMT905JN26J262542",
        "data": 100
      }`,
      expectedPath: "Vehicle_Speed",
    },
    {
      name: "Valid subscribe request",
      json: `{
        "type": "subscribe",
        "schema": "Vehicle",
        "instance": "SMT905JN26J262542"
      }`,
      expectedPath: "Vehicle",
    },
    {
      name: "Valid unsubscribe request",
      json: `{
        "type": "unsubscribe",
        "schema": "Vehicle",
        "instance": "SMT905JN26J262542"
      }`,
      expectedPath: "Vehicle",
    },
    {
      name: "Valid getTimeseries request",
      json: `{
        "type": "timeseries/get",
        "path": "Person.Vitals.Cardiovascular.HeartRate",
        "instance": "P123",
        "query": {
          "gte": { "seconds": 1733389000, "nanos": 814343000 },
          "lte": { "seconds": 1733389389, "nanos": 814343000 }
        }
      }`,
      expectedPath: "Person_Vitals_Cardiovascular_HeartRate",
    }    
  ];

  const invalidRequests = [
    {
      name: "Additional not specified property in get",
      json: `{
        "type": "get",
        "path": "Speed",
        "schema": "Vehicle",
        "instance": "SMT905JN26J262542",
        "extraProperty": "should not be here"
      }`,
      error: "Unexpected property found",
    },
    {
      name: "Missing required property in get",
      json: `{
        "type": "get",
        "path": "Speed",
        "schema": "Vehicle"
      }`,
      error: "The 'instance' field is required",
    },
    {
      name: "Schema is blank",
      json: `{
        "type": "get",
        "path": "Speed",
        "schema": "",
        "instance": "SMT905JN26J262542"
      }`,
      error: "The 'schema' field must contain at least one non-whitespace character",
    },
    {
      name: "Schema contains only whitespace characters",
      json: `{
        "type": "get",
        "path": "Speed",
        "schema": "   ",
        "instance": "SMT905JN26J262542"
      }`,
      error: "The 'schema' field must contain at least one non-whitespace character",
    },
    {
      name: "Instance is blank",
      json: `{
        "type": "get",
        "path": "Speed",
        "instance": "",
        "schema": "SMT905JN26J262542"
      }`,
      error: "The 'instance' field must contain at least one non-whitespace character",
    },
    {
      name: "Instance contains only whitespace characters",
      json: `{
        "type": "get",
        "path": "Speed",
        "instance": "   ",
        "schema": "Vehicle"
      }`,
      error: "The 'instance' field must contain at least one non-whitespace character",
    },
    {
      name: "Unknown request 'type'",
      json: `{
        "type": "unknownType",
        "path": "Speed",
        "schema": "Vehicle",
        "instance": "Vehicle"
      }`,
      error: "Unknown request type: unknownType",
    },
    {
      name: "Missing instance in getTimeseries",
      json: `{
        "type": "timeseries/get",
        "path": "Person.Vitals.Cardiovascular.HeartRate"
      }`,
      error: "The 'instance' field is required",
    },
    {
      name: "Additional property in getTimeseries",
      json: `{
        "type": "timeseries/get",
        "path": "Person.Vitals.Cardiovascular.HeartRate",
        "instance": "P123",
        "extraProperty": "unexpected"
      }`,
      error: "Unexpected property found",
    },
    {
      name: "Invalid query in getTimeseries",
      json: `{
        "type": "timeseries/get",
        "path": "Person.Vitals.Cardiovascular.HeartRate",
        "instance": "P123",
        "query": {
          "gte": { "seconds": "invalid", "nanos": 814343000 }
        }
      }`,
      error: "must be number",
    }
  ];

  describe("Valid Requests", () => {
    validRequests.forEach(({ name, json, expectedPath }) => {
      test(name, () => {
        const result = validator.validate(json);
        expect(result.valid).toBe(true);
        expect(result).toHaveProperty("message");
        expect(result.message).toHaveProperty("path", expectedPath)

      });
    });
  });

  describe("Invalid Requests", () => {
    invalidRequests.forEach(({ name, json, error }) => {
      test(name, () => {
        const result = validator.validate(json);
  
        // Ensure the result is invalid before accessing errors
        if (result.valid === false) {
          expect(result.errors).toEqual(expect.arrayContaining([error]));
        } else {
          // Fail the test if the result is valid when it shouldn't be
          throw new Error(`Expected validation to fail, but it passed. Result: ${JSON.stringify(result)}`);
        }
      });
    });
  });
  
});
