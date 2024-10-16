set(CMAKE_CXX_STANDARD 17)

if(MSVC)
  #this is to make peglib happy for msvc build. otherwise it seems to complain that __cplusplus macro is not set as C++17
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /utf-8")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(CPP_PEGLIB_INCLUDE_DIR ./dependencies/cpp-peglib)
set(JSON_INCLUDE_DIR ./dependencies/json)
set(ANGELSCRIPT_DIR ./dependencies/angelscript_2.36.1)

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

add_library(LegitScript
  ${legit_script_sources}
  ${dep_sources}
)
target_include_directories(LegitScript PRIVATE "${CPP_PEGLIB_INCLUDE_DIR}")
target_include_directories(LegitScript PRIVATE "${ANGELSCRIPT_DIR}/include")
target_include_directories(LegitScript PRIVATE "${ANGELSCRIPT_DIR}/add_on")
target_include_directories(LegitScript PRIVATE "${JSON_INCLUDE_DIR}")

if (EMSCRIPTEN)
  target_link_options(LegitScript PRIVATE
    -fexceptions
  )

  target_compile_options(LegitScript PRIVATE
    -O2
    -fexceptions
  )
endif()
