#pragma once
#include "ScriptParser.h"
#include "../include/LegitScriptCalls.h"
#include <functional>

namespace ls
{


  struct RenderGraphScript
  {
    RenderGraphScript(SliderFloatFunc slider_float_func, SliderIntFunc slider_int_func, TextFunc text_func);
    ~RenderGraphScript();
    void LoadScript(std::string script_src, const std::vector<ls::PassDecl> &pass_decls);
    ScriptCalls RunScript(ivec2 swapchain_size, float time);
    
  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
  };


}