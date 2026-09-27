#pragma once

#include <cstdint>

#include "Sensors.h"
#include "CMD.h"
#include "Telemetry.h"
#include "Test.h"

namespace Msg{
  enum class MsgType: uint16_t{
    //sensors
    IMU,
    BAROMETER,
    POWER_MONITOR,
    MAGNETOMETER,
    GNSS,
    
    //telemetry
    CTR_TELEMETRY,
    PQ_TELEMETRY,
    
    //commands
    CTR_CMD,
    PQ_CMD,
    PQ_IMG_STB,
    PQ_SCI_EXP,
    
    //test
    TEST,
  };
  
  
  //map msg type
  //no default value for compile error check of msg validity
  template <typename T>
  struct IDof;
  
  //SENSOR
  template <>
  struct IDof<Sensor::IMU>{
    static constexpr MsgType type = MsgType::IMU;
  };
  template <>
  struct IDof<Sensor::Barometer>{
    static constexpr MsgType type = MsgType::BAROMETER;
  };
  template <>
  struct IDof<Sensor::PowerMonitor>{
    static constexpr MsgType type = MsgType::POWER_MONITOR;
  };
  template <>
  struct IDof<Sensor::Magnetometer>{
    static constexpr MsgType type = MsgType::MAGNETOMETER;
  };
  template <>
  struct IDof<Sensor::GNSS>{
    static constexpr MsgType type = MsgType::GNSS;
  };
  
  
  //TELEMETRY
  template <>
  struct IDof<Telemetry::CTR>{
    static constexpr MsgType type = MsgType::CTR_TELEMETRY;
  };
  template <>
  struct IDof<Telemetry::PQ>{
    static constexpr MsgType type = MsgType::PQ_TELEMETRY;
  };
  
  
  //CMD
  template <>
  struct IDof<CMD::CTR>{
    static constexpr MsgType type = MsgType::CTR_CMD;
  };
  template <>
  struct IDof<CMD::PQ>{
    static constexpr MsgType type = MsgType::PQ_CMD;
  };
  template <>
  struct IDof<CMD::PQ_IMG_STB>{
    static constexpr MsgType type = MsgType::PQ_IMG_STB;
  };
  template <>
  struct IDof<CMD::PQ_SCI_EXP>{
    static constexpr MsgType type = MsgType::PQ_SCI_EXP;
  };
  
  //Test
  template <>
  struct IDof<Test::MsgTest>{
    static constexpr MsgType type = MsgType::TEST;
  };
}//namespace Msg
