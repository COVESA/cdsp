const { IoTDBDataType } = require("./IoTDBConstants");
class IoTDBRpcDataSet {
  // Static properties
  static TIMESTAMP_STR = "Time";
  static START_INDEX = 2;
  static FLAG = 0x80;

  // Private fields
  #sql;
  #columnNameList;
  #columnTypeList;
  #queryId;
  #client;
  #statementId;
  #sessionId;
  #queryDataSet;
  #ignoreTimestamp;
  #fetchSize;
  #columnOrdinalDict;
  #columnTypeDeduplicatedList;
  #timeBytes;
  #currentBitmap;
  #hasCachedRecord;
  #value;
  #emptyResultSet;
  #rowsIndex;
  #columnSize;

  constructor(
    columnNameList,
    columnTypeList,
    columnNameIndex,
    queryId,
    client,
    statementId,
    sessionId,
    queryDataSet,
    ignoreTimestamp,
    fetchSize
  ) {
    this.#columnNameList = [];
    this.#columnTypeList = [];
    this.#queryId = queryId;
    this.#client = client;
    this.#statementId = statementId;
    this.#sessionId = sessionId;
    this.#queryDataSet = queryDataSet;
    this.#ignoreTimestamp = ignoreTimestamp;
    this.#fetchSize = fetchSize;
    this.#columnSize = columnNameList.length;
    this.#columnOrdinalDict = new Map();
    this.#columnTypeDeduplicatedList = [];
    this.#hasCachedRecord = false;
    this.#emptyResultSet = false;
    this.#rowsIndex = 0;

    if (!ignoreTimestamp) {
      this.#columnNameList.push(IoTDBRpcDataSet.TIMESTAMP_STR);
      this.#columnTypeList.push(IoTDBDataType.INT64);
      this.#columnOrdinalDict.set(IoTDBRpcDataSet.TIMESTAMP_STR, 1);
    }

    if (columnNameIndex !== null) {
      // Initialize deduplicated list
      for (let j = 0; j < columnNameIndex.length; j++) {
        this.#columnTypeDeduplicatedList.push(null);
      }

      // Populate column_name_list and column_type_list
      for (let i = 0; i < columnNameList.length; i++) {
        const name = columnNameList[i];
        this.#columnNameList.push(name);
        this.#columnTypeList.push(IoTDBDataType[columnTypeList[i]]);
        if (!this.#columnOrdinalDict.has(name)) {
          let index = columnNameIndex[name];
          this.#columnOrdinalDict.set(
            name,
            index + IoTDBRpcDataSet.START_INDEX
          );
          this.#columnTypeDeduplicatedList[index] =
            IoTDBDataType[columnTypeList[i]];
        }
      }
    }

    this.#timeBytes = Buffer.alloc(0);
    this.#currentBitmap = Array(this.#columnTypeDeduplicatedList.length).fill(
      Buffer.alloc(0)
    );
    this.#value = Array(this.#columnTypeDeduplicatedList.length).fill(null);
  }

  /**
   * Advances the cursor to the next row in the result set.
   *
   * @returns {boolean} - Returns true if the cursor was successfully advanced to the next row,
   *                      or if there are no more rows in the result set. Returns false if
   *                      there are no more rows and no cached results.
   */
  next() {
    if (this.#hasCachedResult()) {
      this.#constructOneRow();
      return true;
    }
    if (this.#emptyResultSet) {
      return true;
    }
    if (this.#fetchResults()) {
      this.#constructOneRow();
      return true;
    }
    return false;
  }

  getHasCachedRecord() {
    return this.#hasCachedRecord;
  }

  setHasCachedRecord(value) {
    this.#hasCachedRecord = value;
  }

  getTimeBytes() {
    return this.#timeBytes;
  }

  getColumnSize() {
    return this.#columnSize;
  }

  getIgnoreTimestamp() {
    return this.#ignoreTimestamp;
  }

  getColumnNames() {
    return this.#columnNameList;
  }

  getColumnOrdinalDict() {
    return this.#columnOrdinalDict;
  }

  /**
   * Checks if the value at the specified column index is null.
   *
   * @param {number} columnIndex - The index of the column to check.
   * @returns {boolean} - Returns true if the value at the specified column index is null, otherwise false.
   */
  isNullByIndex(columnIndex) {
    let index =
      this.#columnOrdinalDict.get(this.findColumnNameByIndex(columnIndex)) -
      IoTDBRpcDataSet.START_INDEX;

    // time column will never be null
    if (index < 0) {
      return true;
    }
    return this.#isNull(index, this.#rowsIndex - 1);
  }

  getValues() {
    return this.#value;
  }

  getColumnTypeDeduplicatedList() {
    return this.#columnTypeDeduplicatedList;
  }

  /**
   * Finds the column name by its index.
   * @param {number} columnIndex - The index of the column (starting from 1).
   * @returns {string} The name of the column corresponding to the index.
   * @throws {Error} If the column index is less than or equal to 0 or greater than the number of columns.
   */
  findColumnNameByIndex(columnIndex) {
    if (columnIndex <= 0) {
      throw new Error("Column index should start from 1");
    }
    if (columnIndex > this.#columnNameList.length) {
      throw new Error("Column index out of range");
    }
    return this.#columnNameList[columnIndex - 1];
  }

  /**
   * Checks if there is a cached result available.
   *
   * @returns {boolean} True if there is a cached result, otherwise false.
   */
  #hasCachedResult() {
    return this.#queryDataSet !== null && this.#queryDataSet.time.length !== 0;
  }

  /**
   * Constructs one row of data by reading from the dataset buffers.
   * This method updates the internal state of the object by reading
   * time, bitmap, and value buffers, and discarding the bytes that have been read.
   */
  #constructOneRow() {
    // Simulating buffer, read 8 bytes from data set and discard first 8 bytes which have been read.
    this.#timeBytes = this.#queryDataSet.time.slice(0, 8);
    this.#queryDataSet.time = this.#queryDataSet.time.slice(8);

    for (let i = 0; i < this.#queryDataSet.bitmapList.length; i++) {
      let bitmapBuffer = this.#queryDataSet.bitmapList[i];

      // Another 8 new rows, should move the bitmap buffer position to next byte.
      if (this.#rowsIndex % 8 === 0) {
        this.#currentBitmap[i] = bitmapBuffer[0];
        this.#queryDataSet.bitmapList[i] = bitmapBuffer.slice(1);
      }

      if (!this.#isNull(i, this.#rowsIndex)) {
        let valueBuffer = this.#queryDataSet.valueList[i];
        let dataType = this.#columnTypeDeduplicatedList[i];

        // Simulating buffer based on data type.
        switch (dataType) {
          case IoTDBDataType.BOOLEAN:
            this.#value[i] = valueBuffer.slice(0, 1);
            this.#queryDataSet.valueList[i] = valueBuffer.slice(1);
            break;
          case IoTDBDataType.INT32:
            this.#value[i] = valueBuffer.slice(0, 4);
            this.#queryDataSet.valueList[i] = valueBuffer.slice(4);
            break;
          case IoTDBDataType.INT64:
            this.#value[i] = valueBuffer.slice(0, 8);
            this.#queryDataSet.valueList[i] = valueBuffer.slice(8);
            break;
          case IoTDBDataType.FLOAT:
            this.#value[i] = valueBuffer.slice(0, 4);
            this.#queryDataSet.valueList[i] = valueBuffer.slice(4);
            break;
          case IoTDBDataType.DOUBLE:
            this.#value[i] = valueBuffer.slice(0, 8);
            this.#queryDataSet.valueList[i] = valueBuffer.slice(8);
            break;
          case IoTDBDataType.TEXT:
            let length = valueBuffer.readInt32BE(0);
            this.#value[i] = valueBuffer.slice(4, 4 + length);
            this.#queryDataSet.valueList[i] = valueBuffer.slice(4 + length);
            break;
          default:
            throw new Error(`Unsupported data type: ${dataType}`);
        }
      }
    }
    this.#rowsIndex += 1;
    this.#hasCachedRecord = true;
  }

  /**
   * Checks if the value at the specified index and row number is null.
   *
   * @param {number} index - The index in the bitmap array.
   * @param {number} rowNum - The row number to check.
   * @returns {boolean} - Returns true if the value is null, otherwise false.
   */
  #isNull(index, rowNum) {
    let bitmap = this.#currentBitmap[index];
    let shift = rowNum % 8;
    return ((IoTDBRpcDataSet.FLAG >> shift) & (bitmap & 0xff)) === 0;
  }

  #fetchResults() {
    this.#rowsIndex = 0;
  }
}

module.exports = IoTDBRpcDataSet;
