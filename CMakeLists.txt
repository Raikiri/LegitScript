cmake_minimum_required(VERSION 3.29.0)
project(LegitScriptCMake VERSION 0.1.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")


set(CPP_PEGLIB_INCLUDE_DIR ./dependencies/cpp-peglib)
set(ANGELSCRIPT_DIR ./dependencies/angelscript_2.36.1)

include_directories ("${CPP_PEGLIB_INCLUDE_DIR}")
include_directories ("${ANGELSCRIPT_DIR}/include")
include_directories ("${ANGELSCRIPT_DIR}/add_on")


file(GLOB_RECURSE dep_sources ${ANGELSCRIPT_DIR}/source/*.cpp ${ANGELSCRIPT_DIR}/add_on/*.cpp)
file(GLOB_RECURSE legit_script_sources ./source/*.cpp ./source/*.h)
file(GLOB_RECURSE legit_script_headers ./include/*.h)

set(CMAKE_DEBUG_POSTFIX d)

add_executable(LegitScript)

#source_group(TREE src/)
target_sources(LegitScript PRIVATE ${dep_sources} ${legit_script_sources} ${legit_script_headers})
target_link_libraries(LegitScript)
set_target_properties(LegitScript PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/cmaked")
set_target_properties(LegitScript PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/cmake")