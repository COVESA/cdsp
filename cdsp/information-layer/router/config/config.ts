import dotenv from "dotenv";
dotenv.config();

// Define the type of the handler, which will be a lowercase string
export const getHandlerType = (): string => {
  const handlerType = process.env.HANDLER_TYPE;

  if (!handlerType) {
    throw new Error("HANDLER_TYPE must be specified as an ENV variable");
  }

  return handlerType.toLowerCase(); // Ensure it's returned as lowercase
};
