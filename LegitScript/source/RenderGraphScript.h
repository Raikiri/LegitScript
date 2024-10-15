#pragma once
#include "ScriptParser.h"
#include "../include/LegitScriptEvents.h"
#include "../include/LegitScriptCallbacks.h"
#include <functional>

namespace ls
{
  class RenderGraphBuildException : public std::runtime_error
  {
  public:
    RenderGraphBuildException(
      size_t line,
      size_t column,
      std::string desc) :
      line(line),
      column(column),
      desc(desc),
      std::runtime_error(std::string("[") + std::to_string(line) + ":" + std::to_string(column) + "]" + ":" + desc){}

    size_t line;
    size_t column;
    std::string desc;
  };

  class RenderGraphRuntimeException : public std::runtime_error
  {
  public:
    RenderGraphRuntimeException(
      size_t line,
      std::string func,
      std::string desc) :
      line(line),
      func(func),
      desc(desc),
      std::runtime_error(std::string("[") + std::to_string(line) + "]" + func + ":" + desc){}

    size_t line;
    std::string func;
    std::string desc;
  };
  
  struct RenderGraphScript
  {
    RenderGraphScript(SliderFloatFunc slider_float_func, SliderIntFunc slider_int_func, TextFunc text_func);
    ~RenderGraphScript();
    void LoadScript(std::string script_src, const std::vector<ls::PassDecl> &pass_decls);
    ScriptEvents RunScript(ivec2 swapchain_size, float time);
    
  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
  };


}