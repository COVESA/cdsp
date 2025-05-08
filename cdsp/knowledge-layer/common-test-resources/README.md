# Common Test Resources

The **Common Test Resources** module provides a set of reusable components that support testing across the project. This module does not contain direct test cases but instead offers **fixtures, utilities, and helper functions** that facilitate test setup, execution, and validation.

## Purpose
Testing often requires **setup, data generation, and environment configuration** that can be repetitive. To streamline the process, this module centralizes common testing functionalities, ensuring consistency and reducing redundant code.

## Mudule Structure
The module consists of two primary subdirectories:

### 1. **Common Fixtures** (`common_fixtures/`)
Fixtures are used to set up and tear down test environments. They ensure that each test starts with a known, consistent state:
- Handles the initialization of valid test data related to server interactions.

### 2. **Test Utilities** (`utils/`)
Utility functions that provide **data generation, randomization, and formatting** for test cases:

- Generates test data points.
- Handles test observation IDs.
- Generates random values for testing.
- Provides utilities for handling UTC dates.
- Generates Vehicle identification numbers (VINs) for tests.

## How It Works
1. **Fixtures** are used in test cases to **set up** test environments with valid data before execution and **clean up** afterward.
2. **Utilities** help create randomized or structured test data dynamically.
3. By using these resources, test cases remain **clean, readable, and maintainable** while avoiding code duplication.


> [!NOTE]
> If new test utilities or fixtures are needed, consider adding them here instead of duplicating logic in multiple test files.

