# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "bootloader/bootloader.bin"
  "bootloader/bootloader.elf"
  "bootloader/bootloader.map"
  "config/sdkconfig.cmake"
  "config/sdkconfig.h"
  "esp-idf/mbedtls/x509_crt_bundle"
  "flash_bootloader_args"
  "flasher_args.json"
  "flasher_args.json.in"
  "ldgen_libraries.in_library_msg_test"
  "ldgen_libraries_library_msg_test"
  "x509_crt_bundle.S"
  )
endif()
