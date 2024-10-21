interface Int64 {
    constructor(o?: number | string): this;
    toString(): string;
    toJson(): string;
}

export enum TSProtocolVersion {
    IOTDB_SERVICE_PROTOCOL_V1 = 1,
    IOTDB_SERVICE_PROTOCOL_V2 = 2,
    IOTDB_SERVICE_PROTOCOL_V3 = 3,
}

export class TSOpenSessionReq {
    username?: string;
    password?: string;
    client_protocol?: number;
    zoneId?: string;
    configuration?: Map<string,string>;

    constructor(arg?: {
        username?: string;
        password?: string;
        client_protocol?: number;
        zoneId?: string;
        configuration?: Map<string,string>;
    })
}

export class TSCloseSessionReq {
    sessionId: Int64;

    constructor(arg?: {
        sessionId: Int64;
    })
}

export class TSExecuteStatementReq {
    sessionId: Int64;
    statement: string;
    statementId: Int64;
    fetchSize: number;
    timeout?: number;

    constructor(arg?: {
        sessionId: Int64;
        statement: string;
        statementId: Int64;
        fetchSize: number;
        timeout?: number;
    })
}

export class TSInsertRecordReq {
    sessionId: Int64;
    prefixPath: string;
    measurements: string[];
    values: Buffer;
    timestamp: Int64;
    isAligned?: boolean;

    constructor(arg?: {
        sessionId: Int64;
        prefixPath: string;
        measurements: string[];
        values: Buffer;
        timestamp: Int64;
        isAligned?: boolean;
    })
}

export class TSOpenSessionResp {
    sessionId: Int64;
    serverProtocolVersion: number;

    constructor(arg?: {
        sessionId: Int64;
        serverProtocolVersion: number;
    })
}

export class TSExecuteStatementResp {
    columns?: string[];
    dataTypeList?: number[];
    columnNameIndexMap?: Map<string,number>;
    queryId?: Int64;
    queryDataSet?: QueryDataSet;
    ignoreTimeStamp?: boolean;

    constructor(arg?: {
        columns?: string[];
        dataTypeList?: number[];
        columnNameIndexMap?: Map<string,number>;
        queryId?: Int64;
        queryDataSet?: QueryDataSet;
        ignoreTimeStamp?: boolean;
    })
}

export class QueryDataSet {
    valueList: Buffer[];
    bitmapList: Buffer[];
    time: Buffer;

    constructor(arg?: {
        valueList: Buffer[];
        bitmapList: Buffer[];
        time: Buffer;
    })
}

