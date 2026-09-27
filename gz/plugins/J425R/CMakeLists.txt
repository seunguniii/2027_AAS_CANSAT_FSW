cmake_minimum_required(VERSION 3.10)

project(J425R)

find_package(gz-sim8 REQUIRED)

add_library(J425R SHARED J425R.cc)

target_link_libraries(J425R gz-sim8::gz-sim8)
