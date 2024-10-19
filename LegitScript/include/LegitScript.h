#pragma once
#include <vector>
#include <string>
#include <memory>
#include "LegitScriptEvents.h"
#include "LegitScriptInputs.h"
#include "LegitExceptions.h"

namespace ls
{
  struct LegitScript
  {
    LegitScript();
    ~LegitScript();
    ls::ScriptContents LoadScript(const std::string &script_source);
    ls::ScriptEvents RunScript(const std::vector<ContextInput> &context_inputs);
  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
  };
}