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

