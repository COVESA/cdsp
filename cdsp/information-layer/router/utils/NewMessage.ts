// new API for SEND operations

export enum NewMessageType {
  Get = "get",
  Set = "set",
  Subscribe = "subscribe",
  Unsubscribe = "unsubscribe",
  TimeseriesGet = "timeseries/get",
  PermissionsEdit = "permissions/edit",
}

export type GetMessageType = {
  type: NewMessageType.Get;
  path: string;
  instance: string;
  requestId: string;
  format: "nested" | "flat";
  root: "absolute" | "relative";
};

export type SetMessageType = {
  type: NewMessageType.Set;
  path: string;
  instance: string;
  data: any;
  requestId: string;
  metadata?: Record<string, any>;
  timeseries?: boolean;
};

export type SubscribeMessageType = {
  type: NewMessageType.Subscribe;
  path: string;
  instance: string;
  requestId: string;
  format: "nested" | "flat";
  root: "absolute" | "relative";
};

export type UnsubscribeMessageType = {
  type: NewMessageType.Unsubscribe;
  path: string;
  instance: string;
  requestId: string;
};

export type PermissionsEditMessageType = {
  type: NewMessageType.PermissionsEdit;
  userId: string;
  allow?: string[];
  deny?: string[];
  delete?: string[];
  requestId: string;
};

export type TimeseriesGetMessageType = {
  type: NewMessageType.TimeseriesGet;
  path: string;
  instance: string;
  requestId: string;
  query?: { gte?: Timestamp; lte?: Timestamp };
};

export type Timestamp = {
  seconds: number;
  nanos: number;
};

export type NewMessage =
  | GetMessageType
  | SetMessageType
  | SubscribeMessageType
  | UnsubscribeMessageType
  | PermissionsEditMessageType
  | TimeseriesGetMessageType;

// new API for responses

export type DataContentMessage = {
  type: "data";
  instance: string;
  schema: string;
  data: any;
  requestId?: string;
  metadata?: any;
};

export type StatusMessage = {
  type: "status";
  code: number;
  message: string;
  requestId: string;
  timestamp: {
    seconds: number;
    nanos: number;
  };
};

// type checks

export function ensureMessageType<T extends NewMessage>(
  message: NewMessage,
  expectedType: NewMessageType,
): T {
  if (message.type !== expectedType) {
    throw new Error(
      `Expected message type '${expectedType}', but received: '${message.type}'`,
    );
  }
  return message as T; // TypeScript safely casts here due to the check
}

export type ErrorMessage = {
  type: "error";
  code: number;
  message: string;
  reason: string;
  requestId?: string;
};
export const STATUS_ERRORS = {
  BAD_REQUEST: 400,
  UNAUTHORIZED: 401,
  FORBIDDEN: 403,
  NOT_FOUND: 404,
  INTERNAL_SERVER_ERROR: 500,
  NOT_IMPLEMENTED: 501,
  SERVICE_UNAVAILABLE: 503,
} as const;
export const STATUS_SUCCESS = {
  OK: 200,
};
