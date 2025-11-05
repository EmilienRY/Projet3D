# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/appRayTracingGPU_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/appRayTracingGPU_autogen.dir/ParseCache.txt"
  "appRayTracingGPU_autogen"
  )
endif()
