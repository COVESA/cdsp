import * as IClientRPCService from './IClientRPCService_types';
import {
    TSOpenSessionReq,
    TSOpenSessionResp,
    TSCloseSessionReq,
    TSExecuteStatementReq,
    TSExecuteStatementResp,
    TSInsertRecordReq,
} from './IClientRPCService_types';
type Callback<T, E> = (err: E, resp: T) => void;

interface Int64 {
    constructor(o?: number | string): this;
    toString(): string;
    toJson(): string;
}

export class Client {
    openSession(req: TSOpenSessionReq, callback: Callback<TSOpenSessionResp, Error>): void;
    openSession(req: TSOpenSessionReq): Promise<TSOpenSessionResp>;

    closeSession(req: TSCloseSessionReq, callback: Callback<void, Error>): void;
    closeSession(req: TSCloseSessionReq): Promise<void>;

    executeQueryStatement(req: TSExecuteStatementReq, callback: Callback<TSExecuteStatementResp, Error>): void;
    executeQueryStatement(req: TSExecuteStatementReq): Promise<TSExecuteStatementResp>;

    requestStatementId(sessionId: Int64, callback: Callback<Int64, Error>): void;
    requestStatementId(sessionId: Int64): Promise<Int64>;

    insertRecord(req: TSInsertRecordReq, callback: Callback<number, Error>): void;
    insertRecord(req: TSInsertRecordReq): Promise<number>;
}
