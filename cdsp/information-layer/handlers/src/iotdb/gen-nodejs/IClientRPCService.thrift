namespace js IClientRPCService
namespace java IClientRPCService

enum TSProtocolVersion {
  IOTDB_SERVICE_PROTOCOL_V1 = 1,
  IOTDB_SERVICE_PROTOCOL_V2 = 2,
  IOTDB_SERVICE_PROTOCOL_V3 = 3
}

struct TSOpenSessionReq {
  1: optional string username
  2: optional string password
  3: optional i32 client_protocol
  4: optional string zoneId
  5: optional map<string, string> configuration
}

struct TSCloseSessionReq {
  1: required i64 sessionId
}

struct TSExecuteStatementReq {
  1: required i64 sessionId
  2: required string statement
  3: required i64 statementId
  4: required i32 fetchSize
  5: optional i32 timeout
}

struct TSInsertRecordReq {
  1: required i64 sessionId
  2: required string prefixPath
  3: required list<string> measurements
  4: required binary values
  5: required i64 timestamp
  6: optional bool isAligned
}

struct TSOpenSessionResp {
  1: required i64 sessionId
  2: required i32 serverProtocolVersion
}

struct TSExecuteStatementResp {
  1: optional list<string> columns
  2: optional list<i32> dataTypeList
  3: optional map<string, i32> columnNameIndexMap
  4: optional i64 queryId
  5: optional QueryDataSet queryDataSet
  6: optional bool ignoreTimeStamp
}

struct QueryDataSet {
  1: required list<binary> valueList
  2: required list<binary> bitmapList
  3: required binary time
}

service IClientRPCService {
  TSOpenSessionResp openSession(1: TSOpenSessionReq req)
  void closeSession(1: TSCloseSessionReq req)
  TSExecuteStatementResp executeQueryStatement(1: TSExecuteStatementReq req)
  i64 requestStatementId(1: i64 sessionId)
  i32 insertRecord(1: TSInsertRecordReq req)
}
