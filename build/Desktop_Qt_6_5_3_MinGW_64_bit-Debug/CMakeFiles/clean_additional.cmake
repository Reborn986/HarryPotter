# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\HarryPotter_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\HarryPotter_autogen.dir\\ParseCache.txt"
  "HarryPotter_autogen"
  )
endif()
