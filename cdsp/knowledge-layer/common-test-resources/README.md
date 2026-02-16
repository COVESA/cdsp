# Common Test Resources

The **Common Test Resources** module provides a set of reusable components that support testing across the project. This module does not contain direct test cases but instead offers **fixtures, utilities, and helper functions** that facilitate test setup, execution, and validation.

## Purpose

Testing often requires **setup, data generation, and environment configuration** that can be repetitive. To streamline the process, this module centralizes common testing functionalities, ensuring consistency and reducing redundant code.

## Module Structure

The module consists of two primary subdirectories:

### 1. **Common Fixtures** (`common_fixtures/`)

Fixtures are used to set up and tear down test environments. They ensure that each test starts with a known, consistent state:

- `server_data_fixture.*` builds representative telemetry/server payloads for
  connector and reasoner integration tests.
- `use_case_fixture/` contains a lightweight model hierarchy that mirrors the
  on-disk structure of real use cases.

### 2. **Test Utilities** (`utils/`)

Utility functions that provide **data generation, randomization, and formatting** for test cases:

- `data_points_utils.*` builds canonical data-point collections for subscriptions.
- `observation_id_utils.*` produces deterministic observation IDs for assertions.
- `random_utils.*` centralizes random number/string generation.
- `utc_date_utils.*` formats timestamps and offsets for time-based checks.
- `vin_utils.*` validates and generates VIN strings for vehicle-centric tests.

## How It Works

1. **Fixtures** are used in test cases to **set up** test environments with valid data before execution and **clean up** afterward.
2. **Utilities** help create randomized or structured test data dynamically.
3. By using these resources, test cases remain **clean, readable, and maintainable** while avoiding code duplication.

> [!NOTE]
> If new test utilities or fixtures are needed, consider adding them here instead of duplicating logic in multiple test files.
