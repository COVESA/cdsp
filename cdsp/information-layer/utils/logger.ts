// Define COLORS as an enum
export enum COLORS {
  RESET = "\x1b[0m",
  GREEN = "\x1b[32m",
  BLUE = "\x1b[34m",
  RED = "\x1b[31m",
  PALE_WHITE = "\x1b[37m",
  GREY = "\x1b[90m",
  YELLOW = "\x1b[33m",
  ORANGE = "\x1b[38;5;208m",
  BOLD = "\x1b[1m",
}

// Define the MessageType enum to strictly enforce allowed types
export enum LogMessageType {
  RECEIVED = "RECEIVED",
  SENT = "SENT",
  ERROR = "ERROR",
  WARNING = "WARNING",
  OTHER = "OTHER",
}

// Mapping MessageType to their corresponding colors
const MessageTypeColors: Record<LogMessageType, COLORS> = {
  [LogMessageType.RECEIVED]: COLORS.GREEN,
  [LogMessageType.SENT]: COLORS.BLUE,
  [LogMessageType.ERROR]: COLORS.RED,
  [LogMessageType.WARNING]: COLORS.YELLOW,
  [LogMessageType.OTHER]: COLORS.PALE_WHITE,
};

/**
 * Logs a message to the console with a specified color.
 *
 * @param message - The message to be logged.
 * @param color - The color to apply to the message. Should be a value from the COLORS enum.
 */
export function logWithColor(message: string, color: COLORS): void {
  const dateTimeNow = new Date().toISOString();
  console.log(`\n${color}${dateTimeNow} ${message}${COLORS.RESET}`);
}

/**
 * Logs a message with a specific format including a timestamp, message type, and label.
 *
 * @param featureStr - The main message string to be logged.
 * @param type - The type of the message, which determines the label and color. Defaults to MessageType.OTHER.
 * @param label - An optional label to override the default label for the message type.
 */
export function logMessage(
  featureStr: string,
  type: LogMessageType = LogMessageType.OTHER,
  label: string = ""
): void {
  let color: COLORS = MessageTypeColors[type];
  const dateTimeNow = new Date().toISOString();
  let labelText: string;

  switch (type) {
    case LogMessageType.RECEIVED:
      labelText = "\u2193 ".concat("Client Received ws: ").concat(label);
      break;
    case LogMessageType.SENT:
      labelText = "\u2191 ".concat("Client Sent ws: ").concat(label);
      break;
    case LogMessageType.ERROR:
      labelText = "\u2716 ".concat(label || "Internal Error");
      break;
    case LogMessageType.OTHER:
      labelText = label || "Message";
      color = COLORS.RESET;
      break;
    default:
      labelText = label || "Unknown";
      color = COLORS.RESET;
  }

  const logEntry = `\n${COLORS.PALE_WHITE}${dateTimeNow}${COLORS.RESET} ${color}${labelText}${COLORS.RESET}`;
  if (process.env.NODE_ENV !== 'test') {
    console.log(logEntry);
    console.log(featureStr);
  }
}

/**
 * Logs an error message with a specified feature string.
 *
 * @param featureStr - A string representing the feature or context where the error occurred.
 * @param error - The error object or unknown error to be logged.
 *                If it's an instance of Error, its message will be logged.
 *                Otherwise, 'Unknown error' will be logged.
 */
export function logError(featureStr: string, error: Error | unknown): void {
  const errMsg = featureStr
    .concat(": ")
    .concat(error instanceof Error ? error.message : "Unknown error");
  logMessage(errMsg, LogMessageType.ERROR);
}

/**
 * Logs an error message with a specified error message type.
 *
 * @param errMsg - The error message to be logged.
 * @returns void
 */
export function logErrorStr(errMsg: string): void {
  logMessage(errMsg, LogMessageType.ERROR);
}
