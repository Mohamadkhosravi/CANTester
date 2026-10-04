# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CAN_Test_autogen"
  "CMakeFiles\\CAN_Test_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\CAN_Test_autogen.dir\\ParseCache.txt"
  )
endif()
