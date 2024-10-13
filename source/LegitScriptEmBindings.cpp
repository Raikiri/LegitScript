#include "../include/LegitScriptEmBindings.h"
#include "../include/LegitScript.h"
#include <assert.h>
#include <json.hpp>
namespace ls
{
  using json = nlohmann::json;
  std::unique_ptr<ls::LegitScript> instance;
  void InitScript(SliderFloatFunc slider_float_func, SliderIntFunc slider_int_func, TextFunc text_func)
  {
    instance.reset(new ls::LegitScript(slider_float_func, slider_int_func, text_func));
  }
  
  std::string OutputShaderDescsJson(const ls::ScriptShaderDescs &descs)
  {
    auto arr = json::array();
    for(auto desc : descs)
    {
      json json_desc = {{"name", desc.name}, {"body", desc.body}};
      arr.push_back(json_desc);
    }

    return std::string(arr.dump(2));
  }
  
  std::string LoadScript(std::string script_source)
  {
    assert(instance);
    auto shader_descs = instance->LoadScript(script_source);
    return OutputShaderDescsJson(shader_descs);
  }
  std::string RunScript(int swapchain_width, int swapchain_height, float time)
  {
    return std::string();
  }
}