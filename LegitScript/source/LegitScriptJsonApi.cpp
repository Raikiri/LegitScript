#include "../include/LegitScriptJsonApi.h"
#include "../include/LegitScript.h"
#include <assert.h>
#include <json.hpp>
namespace ls
{
  using json = nlohmann::json;
  std::unique_ptr<ls::LegitScript> instance;
  ls::ScriptContents script_contents;
  
  void InitScript(SliderFloatFunc slider_float_func, SliderIntFunc slider_int_func, TextFunc text_func)
  {
    instance.reset(new ls::LegitScript(slider_float_func, slider_int_func, text_func));
  }
  
  json SerializeSamplers(const std::vector<ls::ShaderDesc::Sampler> samplers)
  {
    auto arr = json::array();
    for(auto sampler : samplers)
    {
      arr.push_back(json::object({{"type", sampler.type}, {"name", sampler.name}}));
    }
    return arr;
  }
  json SerializeUniforms(const std::vector<ls::ShaderDesc::Uniform> uniforms)
  {
    auto arr = json::array();
    for(auto uniform : uniforms)
    {
      arr.push_back(json::object({{"type", uniform.type}, {"name", uniform.name}}));
    }
    return arr;
  }
  json SerializeInouts(const std::vector<ls::ShaderDesc::Inouts> inouts)
  {
    auto arr = json::array();
    for(auto inout : inouts)
    {
      arr.push_back(json::object({{"type", inout.type}, {"name", inout.name}}));
    }
    return arr;
  }
  json SerializeBlockBody(const ls::BlockBody &body)
  {
    return json::object({
      {"start", body.start},
      {"text", body.text}
    });
  }
  json SerializeShaderDescs(const ls::ShaderDescs &descs)
  {
    auto arr = json::array();
    for(auto desc : descs)
    {
      json json_desc;
      json_desc["name"] = desc.name;
      json_desc["body"] = SerializeBlockBody(desc.body);
      json_desc["includes"] = desc.includes;
      json_desc["samplers"] = SerializeSamplers(desc.samplers);
      json_desc["uniforms"] = SerializeUniforms(desc.uniforms);
      json_desc["outs"] = SerializeInouts(desc.outs);
      arr.push_back(json_desc);
    }

    return arr;
  }
  
  json SerializeScriptException(const ls::ScriptException &e)
  {
    auto obj = json::object();
    obj["line"] = e.line;
    obj["column"] = e.column;
    if(e.func != "")
      obj["func"] = e.func;
    obj["desc"] = e.desc;
    
    return json::object({{"error", obj}});
  }
  json SerializeGenericException(const std::string &e)
  {
    return json::object({{"error", e}});
  }
  
  json SerializeDeclarations(const ls::Declarations &decls)
  {
    auto arr = json::array();
    for(auto decl : decls)
    {
      arr.push_back(json::object({
        {"name", decl.name},
        {"body", SerializeBlockBody(decl.body)},
      }));
    }
    return arr;
  }

  std::string LoadScript(std::string script_source)
  {
    assert(instance);
    json res_obj;
    try
    {
      script_contents = instance->LoadScript(script_source);
      res_obj = json::object({
        {"shader_descs", SerializeShaderDescs(script_contents.shader_descs)},
        {"declarations", SerializeDeclarations(script_contents.declarations)}
        });
    }
    catch(const ls::ScriptException &e)
    {
      res_obj = SerializeScriptException(e);
    }
    catch(const std::exception &e)
    {
      res_obj = SerializeGenericException(e.what());
    }
    return res_obj.dump(2);
  }
  
  std::string PixelFormatToStr(ls::PixelFormats format)
  {
    switch(format)
    {
      case ls::PixelFormats::rgba8: return "rgba8"; break;
      case ls::PixelFormats::rgba16f: return "rgba16f"; break;
      case ls::PixelFormats::rgba32f: return "rgba32f"; break;
    }
    assert(0);
  }
  json SerializeCachedImgRequests(const std::vector<ls::CachedImageRequest> &cached_img_requests)
  {
    auto arr = json::array();
    for(const auto &req : cached_img_requests)
    {
      arr.push_back(json::object({
        {"size_x", req.size.x},
        {"size_y", req.size.y},
        {"pixel_format", PixelFormatToStr(req.pixel_format)},
        {"id", req.id}
      }));
    }
    return arr;
  }
  json SerializeVec2(vec2 val)
  {
    return json::object({{"x", val.x}, {"y", val.y}});
  }
  json SerializeVec3(vec3 val)
  {
    return json::object({{"x", val.x}, {"y", val.y}, {"z", val.z}});
  }
  json SerializeVec4(vec4 val)
  {
    return json::object({{"x", val.x}, {"y", val.y}, {"z", val.z}, {"w", val.w}});
  }
  json SerializeIVec2(ivec2 val)
  {
    return json::object({{"x", val.x}, {"y", val.y}});
  }
  json SerializeIVec3(ivec3 val)
  {
    return json::object({{"x", val.x}, {"y", val.y}, {"z", val.z}});
  }
  json SerializeIVec4(ivec4 val)
  {
    return json::object({{"x", val.x}, {"y", val.y}, {"z", val.z}, {"w", val.w}});
  }
  json SerializeUniformVal(void *ptr, size_t size, std::string type)
  {
    if(type == "float"){ assert(size == sizeof(float)); return json::value_type(*(float*)ptr);}
    if(type == "vec2"){ assert(size == sizeof(vec2)); return SerializeVec2(*(vec2*)ptr);}
    if(type == "vec3"){ assert(size == sizeof(vec3)); return SerializeVec3(*(vec3*)ptr);}
    if(type == "vec4"){ assert(size == sizeof(vec4)); return SerializeVec4(*(vec4*)ptr);}
    if(type == "int"){ assert(size == sizeof(int)); return json::value_type(*(int*)ptr);}
    if(type == "ivec2"){ assert(size == sizeof(ivec2)); return SerializeIVec2(*(ivec2*)ptr);}
    if(type == "ivec3"){ assert(size == sizeof(ivec3)); return SerializeIVec3(*(ivec3*)ptr);}
    if(type == "ivec4"){ assert(size == sizeof(ivec4)); return SerializeIVec4(*(ivec4*)ptr);}
    throw std::runtime_error("Unknown type: " + type);
  }
  json SerializeUniform(void *ptr, size_t size, ls::ShaderDesc::Uniform uniform_desc)
  {
    return json::object({
      {"type", uniform_desc.type},
      {"val", SerializeUniformVal(ptr, size, uniform_desc.type)}
    });
  }
  json SerializeUniforms(const std::vector<uint8_t> &uniform_data, const std::vector<ls::ShaderInvocation::UniformValue> &uniform_vals, const std::vector<ls::ShaderDesc::Uniform> &uniform_decls)
  {
    size_t uniform_idx;
    assert(uniform_vals.size() == uniform_decls.size());
    auto arr = json::array();
    for(size_t uniform_idx = 0; uniform_idx < uniform_vals.size(); uniform_idx++)
    {
      auto decl = uniform_decls[uniform_idx];
      auto val = uniform_vals[uniform_idx];
      assert(val.offset + val.size <= uniform_data.size());
      arr.push_back(SerializeUniform((void*)(uniform_data.data() + val.offset), val.size, decl));
    }
    return arr;
  }
  json SerializeImageArray(const std::vector<ls::Image> images)
  {
    auto arr = json::array();
    for(const auto &img : images)
    {
      arr.push_back(json::object({
        {"id", img.id},
        {"mip_start", img.mip_range.x},
        {"mip_end", img.mip_range.y}
      }));
    }
    return arr;
  }
  json SerializeShaderInvocations(const std::vector<ls::ShaderInvocation> &shader_invocations, const ls::ShaderDescs &shader_descs)
  {
    auto arr = json::array();
    for(const auto &inv : shader_invocations)
    {
      ls::ShaderDesc matching_shader_desc;
      bool is_found = false;
      for(const auto &shader_desc : shader_descs)
      {
        if(shader_desc.name == inv.shader_name)
        {
          is_found = true;
          matching_shader_desc = shader_desc;
        }
      }
      if(!is_found)
      {
        std::runtime_error("Can't find invoked shader" + inv.shader_name);
      }
      arr.push_back(json::object({
        {"shader_name", inv.shader_name},
        {"color_attachments", SerializeImageArray(inv.color_attachments)},
        {"image_sampler_bindings", SerializeImageArray(inv.image_sampler_bindings)},
        {"uniforms", SerializeUniforms(inv.uniform_data, inv.uniform_values, matching_shader_desc.uniforms)}
      }));
    }
    return arr;
  }
  json SerializeScriptEvents(const ls::ScriptEvents &script_events, const ls::ShaderDescs &shader_descs)
  {
    return json::object({
      {"cached_img_requests", SerializeCachedImgRequests(script_events.cached_image_requests)},
      {"loaded_img_requests", json::array()},
      {"shader_invocations", SerializeShaderInvocations(script_events.script_shader_invocations, shader_descs)}
    });
  }

  
  std::string RunScript(int swapchain_width, int swapchain_height, float time)
  {
    assert(instance);
    json res_obj;
    try
    {
      auto script_events = instance->RunScript({swapchain_width, swapchain_height}, time);
      res_obj = SerializeScriptEvents(script_events, script_contents.shader_descs);
    }
    catch(const ls::ScriptException &e)
    {
      res_obj = SerializeScriptException(e);
    }
    catch(const std::exception &e)
    {
      res_obj = SerializeGenericException(e.what());
    }
    return res_obj.dump(2);
  }
}