#pragma once
#include <vector>
#include <string>
#include <memory>
#include "LegitScriptEvents.h"
#include "LegitScriptCallbacks.h"
#include "LegitExceptions.h"

namespace ls
{
  struct LegitScript
  {
    LegitScript(SliderFloatFunc slider_float_func, SliderIntFunc slider_int_func, TextFunc text_func);
    ~LegitScript();
    ls::ScriptShaderDescs LoadScript(std::string script_source);
    ls::ScriptEvents RunScript(ivec2 swapchain_size, float time);
  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
  };
}