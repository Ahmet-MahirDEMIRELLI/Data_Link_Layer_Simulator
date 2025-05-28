# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\viba-20242-proje-13_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\viba-20242-proje-13_autogen.dir\\ParseCache.txt"
  "viba-20242-proje-13_autogen"
  )
endif()
