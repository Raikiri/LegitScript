#include <string>
#include <iostream>

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <json.hpp>

#include "LegitScriptJsonApi.h"

std::string LegitScriptLoad(std::string source, emscripten::val imControls) {
  using json = nlohmann::json;
  try
  {
    return ls::LoadScript(source);
  }
  catch(const std::exception &e)
  {
    return json::object({{"Uncaught error: ", e.what()}});
  }
}

std::string LegitScriptFrame(std::string context_inputs) {
  using json = nlohmann::json;
  try
  {
    return ls::RunScript(context_inputs);
  }
  catch(const std::exception &e)
  {
    return json::object({{"Uncaught error: ", e.what()}});
  }
}

EMSCRIPTEN_BINDINGS(LegitScriptEmscriptenApi) {
  emscripten::function("LegitScriptLoad", LegitScriptLoad);
  emscripten::function("LegitScriptFrame", LegitScriptFrame);
};
