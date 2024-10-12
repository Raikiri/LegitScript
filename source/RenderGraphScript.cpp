#include <map>
#include "RenderGraphScript.h"
#include <stdexcept>
#include <algorithm>
#include "AngelscriptWrapper/angelscript-cpp.h"
#include <iostream>

namespace ls
{
std::string ArgTypeToAsType(ls::ArgDesc::ArgType arg_type)
{
  if(std::holds_alternative<ls::DecoratedPodType>(arg_type))
  {
    auto dec_pod_type = std::get<ls::DecoratedPodType>(arg_type);
    auto access_qualifier = dec_pod_type.access_qalifier.value_or(ls::DecoratedPodType::AccessQualifiers::in);
    if(access_qualifier == ls::DecoratedPodType::AccessQualifiers::out)
    {
      return "Image";
    }
    return PodTypeToString(dec_pod_type.type);
  }
  throw std::runtime_error("Can't convert arg type to as type");
}


std::string CreateAsPassFuncDeclaration(const ls::PassDecl &decl)
{
  std::string as_func_decl;
  as_func_decl += PodTypeToString(decl.return_type) + " ";
  as_func_decl += decl.name + " (";
  
  bool is_first_arg = true;
  for(auto arg_desc : decl.arg_descs)
  {
    if(!is_first_arg)
      as_func_decl += ", ";
    as_func_decl += ArgTypeToAsType(arg_desc.type) + " ";
    as_func_decl += arg_desc.name;
    is_first_arg = false;
  }
  as_func_decl += ")";
  return as_func_decl;
}
const Image::Id swapchain_img_id = 0;

void AddScriptInvocationAsArg(ScriptShaderInvocation &invocation, asIScriptGeneric *gen, size_t param_idx, ls::ArgDesc arg_desc)
{
  auto arg_type = arg_desc.type;
  if(std::holds_alternative<ls::DecoratedPodType>(arg_type))
  {
    auto dec_pod_type = std::get<ls::DecoratedPodType>(arg_type);
    auto access_qualifier = dec_pod_type.access_qalifier.value_or(ls::DecoratedPodType::AccessQualifiers::in);
    if(dec_pod_type.access_qalifier.value_or(ls::DecoratedPodType::AccessQualifiers::in) == ls::DecoratedPodType::AccessQualifiers::out)
    {
      auto script_img = *(ls::Image*)gen->GetArgObject(param_idx);
      invocation.color_attachments.push_back(script_img);
    }else
    {
      using PodTypes = ls::DecoratedPodType::PodTypes;
      switch(dec_pod_type.type)
      {
        case PodTypes::float_: invocation.AddUniformValue(          gen->GetArgFloat(param_idx)); break;
        case PodTypes::vec2:   invocation.AddUniformValue( *(vec2*)gen->GetArgObject(param_idx)); break;
        case PodTypes::vec3:   invocation.AddUniformValue( *(vec3*)gen->GetArgObject(param_idx)); break;
        case PodTypes::vec4:   invocation.AddUniformValue( *(vec4*)gen->GetArgObject(param_idx)); break;
        case PodTypes::int_:   invocation.AddUniformValue(     int(gen->GetArgDWord(param_idx))); break;
        case PodTypes::ivec2:  invocation.AddUniformValue(*(ivec2*)gen->GetArgObject(param_idx)); break;
        case PodTypes::ivec3:  invocation.AddUniformValue(*(ivec3*)gen->GetArgObject(param_idx)); break;
        case PodTypes::ivec4:  invocation.AddUniformValue(*(ivec4*)gen->GetArgObject(param_idx)); break;
        default: throw std::runtime_error("Can't convert arg type to pod type"); break;
      } 
    }
  }else
  if(std::holds_alternative<ls::SamplerTypes>(arg_type))
  {
    auto img = *(ls::Image*)gen->GetArgObject(param_idx);
    invocation.image_sampler_bindings.push_back(img);
  }else
  {
    throw std::runtime_error("Can't convert arg type to as type");
  }
}




struct ScriptState
{
  std::map<std::string, int> slider_int_params;
  std::map<std::string, float> slider_float_params;
};

struct RenderGraphScript::Impl
{
  Impl(SliderFloatFunc slider_float_func, SliderIntFunc slider_int_func, TextFunc text_func);
  void LoadScript(std::string script_src, const std::vector<ls::PassDecl> &pass_decls);
  ScriptCalls RunScript(ivec2 swapchain_size, float time);
private:
  void RecreateAsScriptEngine(const std::vector<ls::PassDecl> &pass_decls);
  void RegisterAsScriptPassFunctions(const std::vector<ls::PassDecl> &pass_decls);
  void RegisterAsScriptGlobals();

  struct ImageInfo
  {
    ivec2 size;
    ls::PixelFormats pixel_format;
    size_t GetMipsCount() const
    {
      size_t mips_count = 0;
      auto tmp_size = size;
      for(;tmp_size.x > 1 && tmp_size.y > 1; tmp_size.x /= 2, tmp_size.y /= 2, mips_count++);
      return mips_count;
    }
    ivec2 GetMipSize(int mip_level) const
    {
      return ivec2{size.x >> mip_level, size.y >> mip_level};
    }
  };
  SliderFloatFunc slider_float_func;
  SliderIntFunc slider_int_func;
  TextFunc text_func;

  std::vector<ImageInfo> image_infos;
  std::unique_ptr<as::ScriptEngine> as_script_engine;
  std::optional<asIScriptFunction*> as_script_func;
  ScriptState script_state;
  ScriptCalls script_calls;
};

void RenderGraphScript::LoadScript(std::string script_src, const std::vector<ls::PassDecl> &pass_decls)
{
  impl->LoadScript(script_src, pass_decls);
}
ScriptCalls RenderGraphScript::RunScript(ivec2 swapchain_size, float time)
{
  return impl->RunScript(swapchain_size, time);
}
RenderGraphScript::RenderGraphScript(SliderFloatFunc slider_float_func, SliderIntFunc slider_int_func, TextFunc text_func)
{
  this->impl.reset(new RenderGraphScript::Impl(slider_float_func, slider_int_func, text_func));
}
RenderGraphScript::~RenderGraphScript()
{
}

RenderGraphScript::Impl::Impl(SliderFloatFunc slider_float_func, SliderIntFunc slider_int_func, TextFunc text_func) :
  slider_float_func(slider_float_func),
  slider_int_func(slider_int_func),
  text_func(text_func)
{
}

void RenderGraphScript::Impl::LoadScript(std::string script_src, const std::vector<ls::PassDecl> &pass_decls)
{
  this->as_script_func.reset();
  RecreateAsScriptEngine(pass_decls);
  auto mod = as_script_engine->LoadScript(script_src);
  this->as_script_func = mod->GetFunctionByName("main");
}

ScriptCalls RenderGraphScript::Impl::RunScript(ivec2 swapchain_size, float time)
{
  script_calls = ScriptCalls();

  image_infos.clear();
  image_infos.push_back({swapchain_size, ls::PixelFormats::rgba8});
  try
  {
    if(this->as_script_func)
      as_script_engine->RunScript(this->as_script_func.value());
  }
  catch(const std::exception &e)
  {
    std::cout << "Script runtime exception: " << e.what() << "\n";
  }
  return script_calls;
}
void RenderGraphScript::Impl::RecreateAsScriptEngine(const std::vector<ls::PassDecl> &pass_decls)
{
  this->as_script_engine.reset();
  this->as_script_engine = as::ScriptEngine::Create([](const asSMessageInfo *msg){
    const char *type = "ERR ";
    if( msg->type == asMSGTYPE_WARNING ) 
      type = "WARN";
    else if( msg->type == asMSGTYPE_INFORMATION ) 
      type = "INFO";

    std::printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
  });
  RegisterAsScriptGlobals();
  RegisterAsScriptPassFunctions(pass_decls);
}


void RenderGraphScript::Impl::RegisterAsScriptGlobals()
{
  as_script_engine->RegisterEnum("PixelFormats", {
    {"rgba8", int(ls::PixelFormats::rgba8)},
    {"rgba16f", int(ls::PixelFormats::rgba16f)},
    {"rgba32f", int(ls::PixelFormats::rgba32f)}
  });
  as_script_engine->RegisterType<ls::Image>("Image");
  
  as_script_engine->RegisterGlobalFunction("int SliderInt(string name, int min_val, int max_val, int def_val = 0)", [this](asIScriptGeneric *gen)
  {
    std::string *name = (std::string*)gen->GetArgObject(0);
    
    if(this->script_state.slider_int_params.count(*name) == 0)
    {
      this->script_state.slider_int_params[*name] = gen->GetArgDWord(3);
    }
    int min_val = gen->GetArgDWord(1);
    int max_val = gen->GetArgDWord(2);
    int &curr_val = this->script_state.slider_int_params[*name];
    curr_val = this->slider_int_func(name->c_str(), curr_val, min_val, max_val);
    gen->SetReturnDWord(curr_val);
  });
  as_script_engine->RegisterGlobalFunction("float SliderFloat(string name, float min_val, float max_val, float def_val = 0.0f)", [this](asIScriptGeneric *gen)
  {
    std::string *name = (std::string*)gen->GetArgObject(0);
    
    if(this->script_state.slider_float_params.count(*name) == 0)
    {
      this->script_state.slider_float_params[*name] = gen->GetArgFloat(3);
    }
    float min_val = gen->GetArgFloat(1);
    float max_val = gen->GetArgFloat(2);
    float &curr_val = (this->script_state.slider_float_params[*name]);
    curr_val = this->slider_float_func(name->c_str(), curr_val, min_val, max_val);
    gen->SetReturnFloat(curr_val);
  });
  as_script_engine->RegisterGlobalFunction("void Text(string str)", [this](asIScriptGeneric *gen)
  {
    std::string text = *(std::string*)gen->GetArgObject(0);
    this->text_func(text);
  });
  
  as_script_engine->RegisterGlobalFunction("Image GetImage(int width, int height, PixelFormats pixel_format)", [this](asIScriptGeneric *gen)
  {
    int width = gen->GetArgDWord(0);
    int height = gen->GetArgDWord(1);
    auto pixel_format = ls::PixelFormats(gen->GetArgDWord(2));
    
    ls::CachedImageRequest image_request;
    Image::Id id = this->image_infos.size();
    this->image_infos.push_back({ivec2{width, height}, pixel_format});

    image_request.id = id;
    image_request.pixel_format = pixel_format;
    image_request.size = {width, height};    
    this->script_calls.cached_image_requests.push_back(image_request);

    ls::Image img;
    img.id = id;
    img.mip_range = ivec2{0, int(this->image_infos[id].GetMipsCount())};
    gen->SetReturnObject(&img);
  });
  as_script_engine->RegisterGlobalFunction("Image GetSwapchainImage()", [this](asIScriptGeneric *gen)
  {
    ls::Image img;
    img.id = swapchain_img_id;
    img.mip_range = ivec2{0, 1};
    gen->SetReturnObject(&img);
  });
  as_script_engine->RegisterMethod("Image", "Image GetMip(int mip_level)", [this](asIScriptGeneric *gen)
  {
    auto src_img = *(ls::Image*)gen->GetObject();
    int mip_level = gen->GetArgDWord(0);
    
    ls::Image dst_img;
    dst_img.id = src_img.id;
    int dst_mip = src_img.mip_range.x + mip_level;
    int mip = std::clamp<int>(dst_mip, 0, this->image_infos[src_img.id].GetMipsCount() - 1);
    dst_img.mip_range = ivec2{mip, mip + 1};

    gen->SetReturnObject(&dst_img);
  });
  as_script_engine->RegisterGlobalFunction("void PrintImg(Image img)", [this](asIScriptGeneric *gen)
  {
    auto img = *(ls::Image*)gen->GetArgObject(0);
    std::cout << "Img id: " << img.id << " [" << img.mip_range.x << ", " << img.mip_range.y << "]\n";
  });
}


void RenderGraphScript::Impl::RegisterAsScriptPassFunctions(const std::vector<ls::PassDecl> &pass_decls)
{
  for(const auto &pass_decl : pass_decls)
  {
    std::string as_func_decl = CreateAsPassFuncDeclaration(pass_decl);
    this->as_script_engine->RegisterGlobalFunction(as_func_decl, [this, pass_decl](asIScriptGeneric *gen)
    {
      ScriptShaderInvocation invocation;
      invocation.shader_name = pass_decl.name;
      for(size_t param_idx = 0; param_idx < pass_decl.arg_descs.size(); param_idx++)
      {
        AddScriptInvocationAsArg(invocation, gen, param_idx, pass_decl.arg_descs[param_idx]);
      }
      this->script_calls.script_shader_invocations.push_back(invocation);
    });
  }
}

}