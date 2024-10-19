#pragma once
#include "ScriptParser.h"
#include "../include/LegitScriptEvents.h"
#include "../include/LegitScriptInputs.h"
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
    RenderGraphScript();
    ~RenderGraphScript();
    void LoadScript(std::string script_src, const std::vector<ls::PassDecl> &pass_decls);
    ScriptEvents RunScript(const std::vector<ContextInput> &context_inputs);
    
  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
  };


}