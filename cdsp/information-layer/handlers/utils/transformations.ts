import {METADATA_SUFFIX} from "../src/iotdb/utils/iotdb-constants";

/**
   * Replace all underscores in the string with dots.
   * @param str - The database filed to transform.
   * @returns - The transformed string with underscores replaced by dots.
   */
  export function replaceUnderscoresWithDots(str: string): string {
    return str.replace(/\_/g, ".");
  }

  /**
   * Replace all dots in the string with underscores.
   * @param str - The string to transform.
   * @returns - The transformed string with dots replaced by underscores.
   */
  export function replaceDotsWithUnderscore(str: string): string {
    return str.replace(/\./g, "_");
  }

  /**
   * Takes an array of strings. Replaces all underscores with dots and joins the strings with separator ', '.
   * @param strings - list of strings
   * @returns - transformed and joint string
   */
  export function toResponseFormat(strings: string[]): string {
    return replaceUnderscoresWithDots(strings.join(", "));
  }
  
  export function removeSuffixFromString(string: string, suffix: string): string {
    return string.replace(new RegExp(`${suffix}$`), '');
  }
