export interface MessageBase {
  type: "read" | "write" | "subscribe" | "unsubscribe" | "update";
  id: string;
  tree: string;
  uuid: string;
  dateTime?: string;
  status?: string;
}

export interface MessageWithNode extends MessageBase {
  node: { name: string; value: any };
  nodes?: never; // Ensures 'nodes' cannot be present if 'node' is used
}

export interface MessageWithNodes extends MessageBase {
  nodes: Array<{ name: string; value: any }>;
  node?: never; // Ensures 'node' cannot be present if 'nodes' is used
}

export type Message = MessageWithNode | MessageWithNodes;

export type ErrorMessage = {
  category: string;
  statusCode: number;
  message: string;
};

export const STATUS_ERRORS = {
  BAD_REQUEST: 400,
  NOT_FOUND: 404,
  UNAUTHORIZED: 401,
  FORBIDDEN: 403,
  INTERNAL_SERVER_ERROR: 500,
  SERVICE_UNAVAILABLE: 503,
} as const;

interface databaseParams {
  databaseName: string;
  dataPointId: string;
}

export interface DatabaseParamsRecord {
  [key: string]: databaseParams;
}

export interface WebSocket {
  send: (data: string) => void;
}

export interface DataPointSchema {
  [key: string]: any;
}
