import { ErrorMessage } from "../router/utils/ErrorMessage";

/**
 * Creates an error message object with the specified category, status code, and message.
 *
 * @param category - The category of the error.
 * @param statusCode - The HTTP status code associated with the error.
 * @param message - A descriptive message explaining the error.
 * @returns An ErrorMessage object containing the category, status code, and message.
 */
export const createErrorMessage = (
  category: string,
  statusCode: number,
  message: string,
): ErrorMessage => ({
  category,
  statusCode,
  message,
});
