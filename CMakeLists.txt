cmake_minimum_required(VERSION 3.29.0)
project(LegitScriptCMake VERSION 0.1.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
add_subdirectory(LegitScript)
set(CMAKE_DEBUG_POSTFIX d)

if (EMSCRIPTEN)
  add_subdirectory(web)
else()
  add_subdirectory(tests)
endif()

