#include <string>
#include <iostream>

#include <emscripten/bind.h>
#include <json.hpp>

#include "LegitScriptJsonApi.h"

std::string LegitScriptLoad(std::string source) {

  ls::InitScript([](std::string name, float val, float min_val, float max_val) -> float
    {
      std::cout << "Slider int: " << val << "[" << min_val << ", " << max_val << "]\n";
      return val;
    },
    [](std::string name, int val, int min_val, int max_val) -> int
    {
      std::cout << "Slider float: " << val << "[" << min_val << ", " << max_val << "]\n";
      return val;
    },
    [](std::string text) -> void
    {
      std::cout << "Text: " << text << "\n";
    });

  using json = nlohmann::json;
  try
  {
    return ls::LoadScript(source);
  }
  catch(const std::exception &e)
  {
    return json::object({{"error", e.what()}});
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
    return json::object({{"error", e.what()}});
  }
}

EMSCRIPTEN_BINDINGS(LegitScriptEmscriptenApi) {
  emscripten::function("LegitScriptLoad", LegitScriptLoad);
  emscripten::function("LegitScriptFrame", LegitScriptFrame);
};
