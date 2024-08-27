const COLORS = {
  RESET: "\x1b[0m",
  GREEN: "\x1b[32m",
  BLUE: "\x1b[34m",
  RED: "\x1b[31m",
  PALE_WHITE: "\x1b[37m",
  GREY: "\x1b[90m",
  YELLOW: "\x1b[33m",
  BOLD: "\x1b[1m",
};

const MessageType = {
  RECEIVED: COLORS.GREEN,
  SENT: COLORS.BLUE,
  ERROR: COLORS.RED,
  OTHER: COLORS,
};

function logWithColor(message, color) {
  console.log("\n", color, message, COLORS.RESET);
}

function logMessage(featureStr, type, label = "") {
  let color = type;
  const datetimeNow = new Date().toLocaleString();
  let labelText;
  switch (type) {
    case MessageType.RECEIVED:
      labelText = "\u2193 ".concat(label || "Client Received");
      break;
    case MessageType.SENT:
      labelText = "\u2191 ".concat(label || "Client Sent");
      break;
    case MessageType.ERROR:
      labelText = "\u2716".concat(label || "Internal Error");
      break;
    default:
      if (Object.values(MessageType.OTHER).includes(type)) {
        labelText = label || "Message";
      } else {
        color = COLORS.RESET;
        labelText = label || "777";
      }
      break;
  }
  const logEntry = `\n${COLORS.PALE_WHITE}${datetimeNow}${COLORS.RESET} ${color}${labelText}${COLORS.RESET}`;
  console.log(logEntry);
  console.log(featureStr);
}

module.exports = { logMessage, logWithColor, MessageType, COLORS };
