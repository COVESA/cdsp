const dotenv = require("dotenv");
dotenv.config();

const getHandlerType = () => {
  const handlerType = process.env.HANDLER_TYPE;
  if (!handlerType) {
    throw new Error("HANDLER_TYPE must be specified as an ENV variable");
  }
  return handlerType.toLowerCase();
};

module.exports = {
  getHandlerType,
};
