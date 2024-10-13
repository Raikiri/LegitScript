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
  
  json SerializeSamplers(const std::vector<ls::ScriptShaderDesc::Sampler> samplers)
  {
    auto arr = json::array();
    for(auto sampler : samplers)
    {
      arr.push_back(json::object({{"type", sampler.type}, {"name", sampler.name}}));
    }
    return arr;
  }
  json SerializeUniforms(const std::vector<ls::ScriptShaderDesc::Uniform> uniforms)
  {
    auto arr = json::array();
    for(auto uniform : uniforms)
    {
      arr.push_back(json::object({{"type", uniform.type}, {"name", uniform.name}}));
    }
    return arr;
  }
  json SerializeInouts(const std::vector<ls::ScriptShaderDesc::Inouts> inouts)
  {
    auto arr = json::array();
    for(auto inout : inouts)
    {
      arr.push_back(json::object({{"type", inout.type}, {"name", inout.name}}));
    }
    return arr;
  }
  std::string OutputShaderDescsJson(const ls::ScriptShaderDescs &descs)
  {
    auto arr = json::array();
    for(auto desc : descs)
    {
      json json_desc;
      json_desc["name"] = desc.name;
      json_desc["body"] = desc.body;
      json_desc["samplers"] = SerializeSamplers(desc.samplers);
      json_desc["uniforms"] = SerializeUniforms(desc.uniforms);
      json_desc["outs"] = SerializeInouts(desc.outs);
      arr.push_back(json_desc);
    }

    json shader_descs_json;
    shader_descs_json["shader_descs"] = arr;
    return std::string(shader_descs_json.dump(2));
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