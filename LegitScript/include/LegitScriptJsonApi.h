#include "LegitScriptEvents.h"
#include "LegitScriptInputs.h"

namespace ls
{
  std::string LoadScript(const std::string &script_source);
  std::string RunScript(const std::string &context_inputs);
}