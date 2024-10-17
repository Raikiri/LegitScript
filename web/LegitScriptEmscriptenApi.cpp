#include <string>
#include <iostream>

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <json.hpp>

#include "LegitScriptJsonApi.h"

std::string LegitScriptLoad(std::string source, emscripten::val imControls) {
  ls::InitScript([imControls](std::string name, float val, float min_val, float max_val) -> float
    {
      return imControls.call<float>("floatSlider", name, val, min_val, max_val);
    },
    [imControls](std::string name, int val, int min_val, int max_val) -> int
    {
      return imControls.call<int>("intSlider", name, val, min_val, max_val);
    },
    [imControls](std::string text) -> void
    {
      imControls.call<void>("text", text);
    });

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

std::string LegitScriptFrame(int swapchain_width, int swapchain_height, float time) {
  using json = nlohmann::json;
  try
  {
    return ls::RunScript(swapchain_width, swapchain_height, time);
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
