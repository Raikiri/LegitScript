set(CMAKE_CXX_STANDARD 17)

set(LEGIT_SCRIPT_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/../LegitScript/include)

add_executable(LegitScriptTest Tests.cpp)
target_include_directories(LegitScriptTest PRIVATE "${LEGIT_SCRIPT_INCLUDE_DIR}")
target_link_libraries(LegitScriptTest PRIVATE LegitScript)

set_target_properties(LegitScriptTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/cmaked")
set_target_properties(LegitScriptTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/cmake")
