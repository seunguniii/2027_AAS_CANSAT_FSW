#include "include/sensors/IMU.h"

namespace PocketQube{
  class FlightLogic{
    Sensros::IMU imu;
    
    imu.timestamp = 1;
    imu.accel = [1, 2, 3];
    imu.rot_rate = [1, 2, 3];
    imu.q = [1, 2, 3, 4];
  };
}
