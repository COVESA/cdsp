/**
 * Creates an error message object.
 * @param {string} type - The type of the error.
 * @param {number} errorCode - The error code associated with the error.
 * @param {string|Object} error - A descriptive error message.
 * @returns {Object} An object containing the error details.
 */
function createErrorMessage(type, errorCode, error) {
  return { type: `${type}:status`, errorCode, error };
}

module.exports = { createErrorMessage };
