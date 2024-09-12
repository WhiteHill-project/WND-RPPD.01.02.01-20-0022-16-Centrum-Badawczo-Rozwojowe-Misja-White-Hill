#pragma once
#include <vector>
#include <string>
struct CaptureData
{
    // Single 'frame'/picture/measurement
    std::string m_server_path;
};
struct CalibrationData
{
    std::string m_server_path;
};
struct SensorPosition
{
    float height;
    float tilt;
    float rotation;
};
struct SensorMeasurement
{
    SensorPosition m_capture_location;
    CalibrationData m_calibration;
    std::vector<CaptureData> m_data;
};
struct DataSubset
{
    std::vector<SensorMeasurement> m_sensors;
};
// GPS + 3d map location data
struct PlatformPosition
{
    float gps_x,gps_y,gps_z;
};
class DataSet
{
    std::vector<DataSubset> m_subsets;
    PlatformPosition m_position;

public:
    bool hasPosition();
};
