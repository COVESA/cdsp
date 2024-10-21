// Implementing Int64 class that matches the structure expected by Thrift
import { Int64 } from "../gen-nodejs/IClientRPCService_types";

/**
 * Represents a 64-bit integer for IoT applications.
 * Implements the Int64 interface.
 */
export class IotDBInt64 implements Int64 {
  private value: bigint; // Holds the 64-bit integer value

  /**
   * Constructs an instance of IotDBInt64.
   * @param input - An optional number or string to initialize the 64-bit integer.
   * If no input is provided, it defaults to 0.
   */
  constructor(input?: number | string) {
    this.value = BigInt(input ?? 0); // Handle input as either number or string
  }

  /**
   * A placeholder constructor method that is not implemented.
   * @param o - An optional number or string parameter.
   * @throws {Error} Throws an error indicating that the method is not implemented.
   */
  ["constructor"](o?: number | string): this {
    throw new Error("Method not implemented.");
  }

  /**
   * Converts the 64-bit integer to its string representation.
   * @returns {string} The string representation of the 64-bit integer.
   */
  toString(): string {
    return this.value.toString();
  }

  /**
   * Converts the 64-bit integer to its JSON representation.
   * @returns {string} The JSON representation of the 64-bit integer.
   */
  toJson(): string {
    return this.toString(); // You can customize this if needed
  }
}
