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

export interface WebSocketWithId extends WebSocket {
  id: string;
}

export interface DataPointSchema {
  [key: string]: any;
}
