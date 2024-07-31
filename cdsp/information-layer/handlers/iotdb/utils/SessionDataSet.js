const JSDataType = require("./IoTDBConstants");
const IoTDBRpcDataSet = require("./IoTDBRpcDataSets");
const { IoTDBDataInterpreter } = require("./IoTDBDataInterpreter");

const dataTypeProcessors = {
  [JSDataType.BOOLEAN]: (bytes) =>
    new Int8Array(new Uint8Array(bytes.slice(0, 1).reverse()).buffer)[0],
  [JSDataType.INT32]: (bytes) =>
    new Int32Array(new Uint8Array(bytes.slice(0, 4).reverse()).buffer)[0],
  [JSDataType.INT64]: (bytes) =>
    new BigInt64Array(new Uint8Array(bytes.slice(0, 8).reverse()).buffer)[0],
  [JSDataType.FLOAT]: (bytes) =>
    new Float32Array(new Uint8Array(bytes.slice(0, 4).reverse()).buffer)[0],
  [JSDataType.DOUBLE]: (bytes) =>
    new Float64Array(new Uint8Array(bytes.slice(0, 8).reverse()).buffer)[0],
  [JSDataType.TEXT]: (bytes) => bytes.toString(),
};

class SessionDataSet {
  /**
   * Constructor for initializing the class with the provided parameters.
   *
   * @param {Array} columnNameList - List of column names.
   * @param {Array} columnTypeList - List of column types.
   * @param {Object} columnNameIndex - Index mapping of column names.
   * @param {string} queryId - Unique identifier for the query.
   * @param {Object} client - Client instance for database connection.
   * @param {string} statementId - Unique identifier for the statement.
   * @param {string} sessionId - Unique identifier for the session.
   * @param {Object} queryDataSet - Data set returned from the query.
   * @param {boolean} ignoreTimestamp - Flag to determine if timestamps should be ignored.
   */
  constructor(
    columnNameList,
    columnTypeList,
    columnNameIndex,
    queryId,
    client,
    statementId,
    sessionId,
    queryDataSet,
    ignoreTimestamp
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

  /**
   * Checks if there is a next element in the IoTDB RPC DataSet.
   *
   * @returns {boolean} True if there is a next element, otherwise false.
   */
  hasNext() {
    return this.iotdbRpcDataSet.next();
  }

  /**
   * Retrieves the next row record from the dataset.
   * If there is no cached record, checks if there is a next record available.
   * If there is no next record available, returns null.
   * Resets the cached record flag after retrieving the record.
   * @returns {Object|null} The next row record or null if no more records are available.
   */
  next() {
    if (!this.iotdbRpcDataSet.getHasCachedRecord()) {
      if (!this.hasNext()) {
        return null;
      }
    }
    this.iotdbRpcDataSet.setHasCachedRecord(false);

    return this.constructRowRecordFromValueArray();
  }

  /**
   * Constructs a row record from a value array.
   *
   * This function processes the IoTDB RPC dataset to extract and convert
   * the timestamp and column values into a structured object.
   *
   * @returns {Object} The constructed row record with timestamp and column values.
   */
  constructRowRecordFromValueArray() {
    let time64 = IoTDBDataInterpreter.extractTimestamp(
      this.iotdbRpcDataSet.getTimeBytes()
    );
    let obj = { timestamp: time64 };

    for (let i = 0; i < this.iotdbRpcDataSet.getColumnSize(); ++i) {
      let index = i + 1;
      let dataSetColumnIndex = i + IoTDBRpcDataSet.START_INDEX;

      if (this.iotdbRpcDataSet.getIgnoreTimestamp()) {
        index -= 1;
        dataSetColumnIndex -= 1;
      }

      let columnName = this.iotdbRpcDataSet.getColumnNames()[index];
      let location =
        this.iotdbRpcDataSet.getColumnOrdinalDict().get(columnName) -
        IoTDBRpcDataSet.START_INDEX;

      if (!this.iotdbRpcDataSet.isNullByIndex(dataSetColumnIndex)) {
        let valueBytes = this.iotdbRpcDataSet.getValues()[location];
        let dataType =
          this.iotdbRpcDataSet.getColumnTypeDeduplicatedList()[location];
        let tsName =
          this.iotdbRpcDataSet.findColumnNameByIndex(dataSetColumnIndex);

        if (dataTypeProcessors[dataType]) {
          obj[tsName] = dataTypeProcessors[dataType](valueBytes);
        } else {
          throw new Error("Unsupported data type.");
        }
      } else {
        let tsName =
          this.iotdbRpcDataSet.findColumnNameByIndex(dataSetColumnIndex);
        obj[tsName] = null;
      }
    }
    return obj;
  }
}

module.exports = SessionDataSet;
