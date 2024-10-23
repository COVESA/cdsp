import { IoTDBDataType } from "./iotdb-constants";
import { IoTDBRpcDataSet } from "./IoTDBRpcDataSet";
import { IoTDBDataInterpreter } from "./IoTDBDataInterpreter";

// Define the dataTypeProcessors map with specific function types
const dataTypeProcessors: {
  [key in IoTDBDataType]: (
    bytes: Uint8Array | Buffer
  ) => number | bigint | string;
} = {
  [IoTDBDataType.BOOLEAN]: (bytes) =>
    new Int8Array(new Uint8Array(bytes.slice(0, 1).reverse()).buffer)[0],
  [IoTDBDataType.INT32]: (bytes) =>
    new Int32Array(new Uint8Array(bytes.slice(0, 4).reverse()).buffer)[0],
  [IoTDBDataType.INT64]: (bytes) =>
    new BigInt64Array(new Uint8Array(bytes.slice(0, 8).reverse()).buffer)[0],
  [IoTDBDataType.FLOAT]: (bytes) =>
    new Float32Array(new Uint8Array(bytes.slice(0, 4).reverse()).buffer)[0],
  [IoTDBDataType.DOUBLE]: (bytes) =>
    new Float64Array(new Uint8Array(bytes.slice(0, 8).reverse()).buffer)[0],
  [IoTDBDataType.TEXT]: (bytes) => bytes.toString(),
};

export class SessionDataSet {
  private iotdbRpcDataSet: IoTDBRpcDataSet;

  constructor(
    columnNameList: string[],
    columnTypeList: IoTDBDataType[],
    columnNameIndex: { [key: string]: number },
    queryId: number,
    client: any,
    statementId: number,
    sessionId: any,
    queryDataSet: any,
    ignoreTimestamp: boolean
  ) {
    this.iotdbRpcDataSet = new IoTDBRpcDataSet(
      columnNameList,
      columnTypeList,
      columnNameIndex,
      queryId,
      client,
      statementId,
      sessionId,
      queryDataSet,
      ignoreTimestamp,
      1024 // Buffer size or default value
    );
  }

  hasNext(): boolean {
    return this.iotdbRpcDataSet.next();
  }

  next(): { [key: string]: any } | null {
    if (!this.iotdbRpcDataSet.getHasCachedRecord()) {
      if (!this.hasNext()) {
        return null;
      }
    }
    this.iotdbRpcDataSet.setHasCachedRecord(false);
    return this.constructRowRecordFromValueArray();
  }

  private constructRowRecordFromValueArray(): { [key: string]: any } {
    const time64 = IoTDBDataInterpreter.extractTimestamp(
      this.iotdbRpcDataSet.getTimeBytes()
    );
    const obj: { [key: string]: any } = { timestamp: time64 };

    for (let i = 0; i < this.iotdbRpcDataSet.getColumnSize(); ++i) {
      let index = i + 1;
      let dataSetColumnIndex = i + IoTDBRpcDataSet.START_INDEX;

      if (this.iotdbRpcDataSet.getIgnoreTimestamp()) {
        index -= 1;
        dataSetColumnIndex -= 1;
      }

      const columnName = this.iotdbRpcDataSet.getColumnNames()[index];
      const location =
        this.iotdbRpcDataSet.getColumnOrdinalDict().get(columnName)! -
        IoTDBRpcDataSet.START_INDEX;

      if (!this.iotdbRpcDataSet.isNullByIndex(dataSetColumnIndex)) {
        const valueBytes = this.iotdbRpcDataSet.getValues()[location];
        let dataType =
          this.iotdbRpcDataSet.getColumnTypeDeduplicatedList()[location];
        const tsName =
          this.iotdbRpcDataSet.findColumnNameByIndex(dataSetColumnIndex);

        if (dataType !== null && valueBytes !== null) {
          if (typeof dataType === "string") {
            dataType = IoTDBDataType[dataType as keyof typeof IoTDBDataType];
          }

          if (dataTypeProcessors[dataType]) {
            obj[tsName] = dataTypeProcessors[dataType](valueBytes);
          } else {
            throw new Error(`Unsupported data type: ${dataType}`);
          }
        } else {
          obj[tsName] = null;
        }
      } else {
        const tsName =
          this.iotdbRpcDataSet.findColumnNameByIndex(dataSetColumnIndex);
        obj[tsName] = null;
      }
    }
    return obj;
  }
}