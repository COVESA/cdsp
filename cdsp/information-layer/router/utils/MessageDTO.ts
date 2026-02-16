import { NewMessageType } from "./NewMessage";


export type NewMessageDTO =
  | GetMessageDTO
  | SetMessageDTO
  | SubscribeMessageDTO
  | UnsubscribeMessageDTO;

type Params = {
  instance: string;
  schema: string;
  path: string;
  format?: "nested" | "flat";
  root?: "absolute" | "relative";
};

export type GetMessageDTO = {
  jsonrpc: string;
  method: NewMessageType.Get;
  id: string;
  params: Params;
};

export type SetMessageDTO = {
  jsonrpc: string;
  method: NewMessageType.Set;
  id: string;
  params: Params & {
    data: any;
    metadata?: Record<string, any>;
    timeseries?: boolean;
  };
};

export type SubscribeMessageDTO = {
  jsonrpc: string;
  method: NewMessageType.Subscribe;
  id: string;
  params: Params;
};

export type UnsubscribeMessageDTO = {
  jsonrpc: string;
  method: NewMessageType.Unsubscribe;
  id: string;
  params: Params;
};

export type DataResponseDTO = {
  jsonrpc: string;
  id: string | null;
  result: {
    instance: string;
    data: any;
    metadata?: any;
  }
};

export type OkResponseDTO = {
  jsonrpc: string;
  id: string;
  result: {}
};

export type ErrorResponseDTO = {
  jsonrpc: string;
  id: string | null;
  error: {
    code: number;
    message: string;
    data: {
      reason: string;
    };
  };
};
