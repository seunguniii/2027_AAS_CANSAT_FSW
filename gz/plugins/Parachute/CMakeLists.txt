cmake_minimum_required(VERSION 3.10)

project(Parachute)

find_package(gz-sim8 REQUIRED)
find_package(gz-transport13 REQUIRED)
find_package(gz-msgs10 REQUIRED)

add_library(Parachute SHARED Parachute.cc)

target_link_libraries(Parachute
  gz-sim8::gz-sim8
  gz-transport13::gz-transport13
  gz-msgs10::gz-msgs10
)
