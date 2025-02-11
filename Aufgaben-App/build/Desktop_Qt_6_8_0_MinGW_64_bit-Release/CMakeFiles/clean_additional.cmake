# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "Aufgaben-App_autogen"
  "CMakeFiles\\Aufgaben-App_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\Aufgaben-App_autogen.dir\\ParseCache.txt"
  )
endif()
