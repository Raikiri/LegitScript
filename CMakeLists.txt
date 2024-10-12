cmake_minimum_required(VERSION 3.29.0)
project(LegitScriptCMake VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
  #this is to make peglib happy for msvc build. otherwise it seems to complain that __cplusplus macro is not set as C++17
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /utf-8")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")


set(CPP_PEGLIB_INCLUDE_DIR ./dependencies/cpp-peglib)
set(ANGELSCRIPT_DIR ./dependencies/angelscript_2.36.1)

include_directories ("${CPP_PEGLIB_INCLUDE_DIR}")
include_directories ("${ANGELSCRIPT_DIR}/include")
include_directories ("${ANGELSCRIPT_DIR}/add_on")


file(GLOB_RECURSE dep_sources ${ANGELSCRIPT_DIR}/source/*.cpp ${ANGELSCRIPT_DIR}/add_on/*.cpp)

#angelscript requires this annoying asm file when compiled under MSVC
#this comes from angelscript's sdk/angelscript/projects/cmake
if(MSVC AND CMAKE_CL_64)
    enable_language(ASM_MASM)
    if(CMAKE_ASM_MASM_COMPILER_WORKS)
        set(dep_sources ${dep_sources} ${ANGELSCRIPT_DIR}/source/as_callfunc_x64_msvc_asm.asm)
    else()
        message(FATAL ERROR "MSVC x86_64 target requires a working assembler")
    endif()
endif()

file(GLOB_RECURSE legit_script_sources ./source/*.cpp ./source/*.h)
file(GLOB_RECURSE legit_script_headers ./include/*.h)
set(CMAKE_DEBUG_POSTFIX d)

add_executable(LegitScript)
#set_property(TARGET LegitScript PROPERTY CXX_STANDARD 20)

add_compile_definitions(COMPILE_TESTS_MAIN)

#source_group(TREE src/)
target_sources(LegitScript PRIVATE ${dep_sources} ${legit_script_sources} ${legit_script_headers})
target_link_libraries(LegitScript)
set_target_properties(LegitScript PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/cmaked")
set_target_properties(LegitScript PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/cmake")