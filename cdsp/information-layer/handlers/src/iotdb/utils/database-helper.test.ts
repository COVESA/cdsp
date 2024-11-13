import { transformSessionDataSet } from './database-helper';
import { SessionDataSet } from './SessionDataSet';

describe('database-helper', () => {
  let mockSessionDataSet: jest.Mocked<SessionDataSet>;

  beforeEach(() => {
    mockSessionDataSet = {
      hasNext: jest.fn(),
      next: jest.fn(),
    } as unknown as jest.Mocked<SessionDataSet>;
  });

  test('transforms SessionDataSet to array of name/value pairs', () => {
    const mockDataRows = [
        { 
            "root.Vehicles_Speed": 60, 
            "root.Vehicles_FuelLevel": 0.8, 
            "root.Vehicles_CurrentLocation_Latitude": 34.052235,
            "root.Vehicles_CurrentLocation_Longitude": -118.243683 
        },
        { 
            "root.Vehicles_Speed": 65, 
            "root.Vehicles_FuelLevel": 0.75, 
            "root.Vehicles_CurrentLocation_Latitude": 34.052240,
            "root.Vehicles_CurrentLocation_Longitude": -118.243680 
        },
        { 
            "root.Vehicles_FuelLevel": 0.71, 
        }
    ];

    let index = 0;
    mockSessionDataSet.hasNext.mockImplementation(() => index < mockDataRows.length);
    mockSessionDataSet.next.mockImplementation(() => mockDataRows[index++]);

    const result = transformSessionDataSet(mockSessionDataSet, 'root.Vehicles');

    expect(result).toEqual([
        { name: "Speed", value: 65 },                          // Latest value for Speed
        { name: "FuelLevel", value: 0.71 },                    // Latest value for FuelLevel
        { name: "CurrentLocation.Latitude", value: 34.052240 },  // Latest Latitude
        { name: "CurrentLocation.Longitude", value: -118.243680 } // Latest Longitude
    ]);
  });
});
