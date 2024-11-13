  /**
   * Transforms a database field name by replacing underscores with dots.
   * @param field - The database filed to transform.
   * @returns - The transformed to message node replacing underscores by dots.
   */
  export function transformDataPointsWithDots(field: string): string {
    return field.replace(/\_/g, ".");
  }

  /**
   * Transforms a message node by replacing dots with underscores.
   * @param node - The message node to transform.
   * @returns - The transformed message node with dots replaced by underscores.
   */
  export function transformDataPointsWithUnderscores(node: string): string {
    return node.replace(/\./g, "_");
  }

