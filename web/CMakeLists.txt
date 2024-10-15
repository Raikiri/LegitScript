set(JSON_INCLUDE_DIR ../LegitScript/dependencies/json)
set(LEGIT_SCRIPT_INCLUDE_DIR ../LegitScript/include)

add_executable(LegitScriptWasm
  ${CMAKE_CURRENT_LIST_DIR}/LegitScriptEmscriptenApi.cpp
)
target_include_directories(LegitScriptWasm PRIVATE "${JSON_INCLUDE_DIR}")
target_include_directories(LegitScriptWasm PRIVATE "${LEGIT_SCRIPT_INCLUDE_DIR}")

target_link_libraries(LegitScriptWasm PUBLIC embind)
target_link_libraries(LegitScriptWasm PRIVATE LegitScript)
target_link_options(LegitScriptWasm PRIVATE
  -sEXPORT_ES6=1
  -sEXCEPTION_CATCHING_ALLOWED=LegitScriptLoad,LegitScriptFrame
)

#install(TARGETS LegitScriptWasm DESTINATION ${CMAKE_CURRENT_LIST_DIR}/dist/)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/LegitScriptWasm.wasm DESTINATION ${CMAKE_CURRENT_LIST_DIR}/dist/)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/LegitScriptWasm.js DESTINATION ${CMAKE_CURRENT_LIST_DIR}/dist/)