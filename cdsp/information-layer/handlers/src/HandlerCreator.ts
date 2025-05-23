import { getHandlerType } from "../config/config";
import { HandlerBase } from "./HandlerBase";
import { IoTDBHandler } from "./iotdb/src/IoTDBHandler";
import { logWithColor, COLORS } from "../../utils/logger";

export function createHandler() : HandlerBase {
    const handlerType: string = getHandlerType();
    logWithColor(`\n ** Handler: ${handlerType} ** \n`, COLORS.BOLD);

    let handler: HandlerBase;

    // Instantiate the correct handler based on the handler type
    switch (handlerType) {
      case "iotdb":
        handler = new IoTDBHandler();
        break;
      default:
        throw new Error("Unsupported handler type");
    }

    return handler;
  }
