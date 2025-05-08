import {transformSessionDataSet} from '../utils/database-helper';
import {SessionDataSet} from '../utils/SessionDataSet';

describe('database-helper',
  () => {
    let mockSessionDataSet: jest.Mocked<SessionDataSet>;

    beforeEach(() => {
      mockSessionDataSet = {
        hasNext: jest.fn(),
        next: jest.fn(),
      } as unknown as jest.Mocked<SessionDataSet>;
    });

    test('transforms SessionDataSet to name/value pairs of data points and metadata', () => {
      const mockDataRows = [
        {
          "timestamp": {"timestamp": BigInt(333333333)},
          "root.Vehicles_Speed": 60,
          "root.Vehicles_FuelLevel": 0.8,
          "root.Vehicles_CurrentLocation_Latitude": 34.052235,
          "root.Vehicles_CurrentLocation_Longitude": -118.243683
        },
        {
          "timestamp": {"timestamp": BigInt(222222222)},
          "root.Vehicles_Speed": 65,
          "root.Vehicles_FuelLevel": 0.75,
          "root.Vehicles_CurrentLocation_Latitude": 34.052240,
          // metadata received timestamp should be amended
          "root.Vehicles_CurrentLocation_Latitude_Metadata": JSON.stringify({timestamps: {generated: {seconds: 999999, nanos: 999999999}}}),
          "root.Vehicles_CurrentLocation_Longitude": -118.243680,
          // metadata received timestamp should overwrite existing received timestamp
          "root.Vehicles_CurrentLocation_Longitude_Metadata": JSON.stringify({timestamps: {received: {seconds: 999999, nanos: 999999999}}}),
        },
        {
          "timestamp": {"timestamp": BigInt(111111111)},
          "root.Vehicles_FuelLevel": 0.71,
        }
      ];

      let index = 0;
      mockSessionDataSet.hasNext.mockImplementation(() => index < mockDataRows.length);
      mockSessionDataSet.next.mockImplementation(() => mockDataRows[index++]);


      const [dataPoints, metadata] = transformSessionDataSet(mockSessionDataSet, 'root.Vehicles');

      expect(dataPoints).toEqual([
        {name: "Speed", value: 65},                          // Latest value for Speed
        {name: "FuelLevel", value: 0.71},                    // Latest value for FuelLevel
        {name: "CurrentLocation.Latitude", value: 34.052240},  // Latest Latitude
        {name: "CurrentLocation.Longitude", value: -118.243680} // Latest Longitude
      ]);

      const expectedTimestampFuelLevel = {timestamps: {received: {seconds: 111111, nanos: 111000000}}}
      const expectedTimestampLatitude = {
        timestamps:
          {
            received: {seconds: 222222, nanos: 222000000},
            generated: {seconds: 999999, nanos: 999999999}
          }
      };
      const expectedTimestampDefault = {timestamps: {received: {seconds: 222222, nanos: 222000000}}}
      expect(metadata).toEqual([
        {name: "Speed", value: expectedTimestampDefault},
        {name: "FuelLevel", value: expectedTimestampFuelLevel},
        {name: "CurrentLocation.Latitude", value: expectedTimestampLatitude},
        {name: "CurrentLocation.Longitude", value: expectedTimestampDefault}
      ]);
    })
    ;
  })
;


