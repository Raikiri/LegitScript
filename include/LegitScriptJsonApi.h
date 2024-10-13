#include "LegitScriptCallbacks.h"

namespace ls
{
  void InitScript(SliderFloatFunc slider_float_func, SliderIntFunc slider_int_func, TextFunc text_func);
  std::string LoadScript(std::string script_source);
  std::string RunScript(int swapchain_width, int swapchain_height, float time);
}