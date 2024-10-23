#include <map>
#include "RenderGraphScript.h"
#include <stdexcept>
#include <algorithm>
#include "AngelscriptWrapper/angelscript-cpp.h"
#include <iostream>
#include <assert.h>

namespace ls
{
std::string ArgTypeToAsTypeSpecific(ls::DecoratedPodType dec_pod_type)
{
  auto access_qualifier = dec_pod_type.access_qalifier.value_or(ls::DecoratedPodType::AccessQualifiers::in);
  if(access_qualifier == ls::DecoratedPodType::AccessQualifiers::out)
  {
    return "Image";
  }
  return PodTypeToString(dec_pod_type.type);
}
std::string ArgTypeToAsTypeSpecific(ls::DecoratedImageType dec_img_type)
{
  return "Image";
}
std::string ArgTypeToAsTypeSpecific(ls::SamplerTypes sampler_type)
{
    return "Image";
}

std::string ArgTypeToAsType(ls::ArgDesc::ArgType arg_type)
{
  return std::visit([](auto arg){
    return ArgTypeToAsTypeSpecific(arg);
  }, arg_type);
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

void AddScriptInvocationAsArgSpecific(ShaderInvocation &invocation, asIScriptGeneric *gen, size_t param_idx, ls::DecoratedPodType dec_pod_type)
{
  auto access_qualifier = dec_pod_type.access_qalifier.value_or(ls::DecoratedPodType::AccessQualifiers::in);
  if(dec_pod_type.access_qalifier.value_or(ls::DecoratedPodType::AccessQualifiers::in) == ls::DecoratedPodType::AccessQualifiers::out)
  {
    auto script_img = *(ls::Image*)gen->GetArgObject(param_idx);
    if(script_img.mip_range.y - script_img.mip_range.x != 1)
    {
      throw ls::RenderGraphRuntimeException(0, "", "Can't bind render target with more than 1 mip");
    }
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
}

void AddScriptInvocationAsArgSpecific(ShaderInvocation &invocation, asIScriptGeneric *gen, size_t param_idx, ls::SamplerTypes sampler_type)
{
  auto img = *(ls::Image*)gen->GetArgObject(param_idx);
  invocation.image_sampler_bindings.push_back(img);
}

void AddScriptInvocationAsArgSpecific(ShaderInvocation &invocation, asIScriptGeneric *gen, size_t param_idx, ls::DecoratedImageType dec_img_type)
{
  throw std::runtime_error("Images are not supported yet");
}

void AddScriptInvocationAsArg(ShaderInvocation &invocation, asIScriptGeneric *gen, size_t param_idx, ls::ArgDesc arg_desc)
{
  std::visit([&invocation, gen, param_idx](auto a){
    AddScriptInvocationAsArgSpecific(invocation, gen, param_idx, a);
  }, arg_desc.type);
}




struct ScriptContext
{
  template<typename VecType>
  VecType &GetContextRef(std::string name);

  std::map<std::string, int> int_params;
  std::map<std::string, ivec2> ivec2_params;
  std::map<std::string, ivec3> ivec3_params;
  std::map<std::string, ivec4> ivec4_params;
  std::map<std::string, float> float_params;
  std::map<std::string, vec2> vec2_params;
  std::map<std::string, vec3> vec3_params;
  std::map<std::string, vec4> vec4_params;
  float curr_time;
};
template<>
float &ScriptContext::GetContextRef<float>(std::string name)
{
  return this->float_params[name];
}
template<>
vec2 &ScriptContext::GetContextRef<vec2>(std::string name)
{
  return this->vec2_params[name];
}
template<>
vec3 &ScriptContext::GetContextRef<vec3>(std::string name)
{
  return this->vec3_params[name];
}
template<>
vec4 &ScriptContext::GetContextRef<vec4>(std::string name)
{
  return this->vec4_params[name];
}
template<>
int &ScriptContext::GetContextRef<int>(std::string name)
{
  return this->int_params[name];
}
template<>
ivec2 &ScriptContext::GetContextRef<ivec2>(std::string name)
{
  return this->ivec2_params[name];
}
template<>
ivec3 &ScriptContext::GetContextRef<ivec3>(std::string name)
{
  return this->ivec3_params[name];
}
template<>
ivec4 &ScriptContext::GetContextRef<ivec4>(std::string name)
{
  return this->ivec4_params[name];
}
template<>
LoadedImage &ScriptContext::GetContextRef<ls::LoadedImage>(std::string name)
{
  static LoadedImage img;
  return img;
}

struct RenderGraphScript::Impl
{
  Impl();
  void LoadScript(std::string script_src, const std::vector<ls::PassDecl> &pass_decls);
  ScriptEvents RunScript(const std::vector<ContextInput> &context_inputs);
private:
  void SetContextInputs(const std::vector<ContextInput> &context_inputs);
  void RecreateAsScriptEngine(const std::vector<ls::PassDecl> &pass_decls);
  void RegisterAsScriptPassFunctions(const std::vector<ls::PassDecl> &pass_decls);
  void RegisterAsScriptGlobals();
  void RegisterImageType();
  template<typename VecType, size_t CompCount>
  void RegisterVecType(std::string type_name, std::string uppercase_type_name, std::string comp_type_name);
  void RegisterBasicTypeOperations();


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
  std::vector<ImageInfo> image_infos;
  std::unique_ptr<as::ScriptEngine> as_script_engine;
  std::optional<asIScriptFunction*> as_script_func;
  ScriptContext script_context;
  ScriptEvents script_events;
};

void RenderGraphScript::LoadScript(std::string script_src, const std::vector<ls::PassDecl> &pass_decls)
{
  impl->LoadScript(script_src, pass_decls);
}
ScriptEvents RenderGraphScript::RunScript(const std::vector<ContextInput> &context_inputs)
{
  return impl->RunScript(context_inputs);
}
RenderGraphScript::RenderGraphScript()
{
  this->impl.reset(new RenderGraphScript::Impl());
}
RenderGraphScript::~RenderGraphScript()
{
}

RenderGraphScript::Impl::Impl()
{
}

void RenderGraphScript::Impl::LoadScript(std::string script_src, const std::vector<ls::PassDecl> &pass_decls)
{
  this->as_script_func.reset();
  RecreateAsScriptEngine(pass_decls);
  auto mod = as_script_engine->LoadScript(script_src);
  this->as_script_func = mod->GetFunctionByName("main");
}

void RenderGraphScript::Impl::SetContextInputs(const std::vector<ContextInput> &context_inputs)
{
  for(const auto &input : context_inputs)
  {
    std::visit([this, &input](auto val){
      this->script_context.GetContextRef<decltype(val)>(input.name) = val;
    }, input.value);
  }
}

ScriptEvents RenderGraphScript::Impl::RunScript(const std::vector<ContextInput> &context_inputs)
{
  script_events = ScriptEvents();
  SetContextInputs(context_inputs);
  
  ivec2 swapchain_size = this->script_context.GetContextRef<ivec2>("@swapchain_size");
  image_infos.clear();
  image_infos.push_back({swapchain_size, ls::PixelFormats::rgba8});
  
  this->script_context.curr_time = this->script_context.GetContextRef<float>("@time");
  
  if(this->as_script_func)
  {
    auto opt_err = as_script_engine->RunScript(this->as_script_func.value());
    if(opt_err)
    {
      throw ls::RenderGraphRuntimeException(
        opt_err->line_number,
        opt_err->func_decl,
        opt_err->exception_str
      );
    }
  }
  else
    throw std::runtime_error("No script loaded");
  return script_events;
}
void RenderGraphScript::Impl::RecreateAsScriptEngine(const std::vector<ls::PassDecl> &pass_decls)
{
  this->as_script_engine.reset();
  this->as_script_engine = as::ScriptEngine::Create(
    [this](const asSMessageInfo *msg){
      std::string msg_str = std::string("[") + std::to_string(msg->row) + ":" + std::to_string(msg->col) + "]" + " " + msg->message;
      if(msg->type == asMSGTYPE_ERROR || msg->type == asMSGTYPE_WARNING)
        throw ls::RenderGraphBuildException(
          msg->row,
          msg->col,
          msg->message);
    }
  );
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
  as_script_engine->RegisterGlobalFunction("int SliderInt(string name, int min_val, int max_val, int def_val = 0)", [this](asIScriptGeneric *gen)
  {
    std::string *name = (std::string*)gen->GetArgObject(0);
    int min_val = gen->GetArgDWord(1);
    int max_val = gen->GetArgDWord(2);
    int def_val = gen->GetArgDWord(3);
    
    if(this->script_context.int_params.count(*name) == 0)
    {
      this->script_context.int_params[*name] = def_val;
    }
    this->script_events.context_requests.push_back(
      IntRequest{*name, min_val, max_val, def_val}
    );
    auto curr_val = this->script_context.GetContextRef<int>(*name);
    gen->SetReturnDWord(curr_val);
  });
  as_script_engine->RegisterGlobalFunction("float SliderFloat(string name, float min_val, float max_val, float def_val = 0.0f)", [this](asIScriptGeneric *gen)
  {
    std::string *name = (std::string*)gen->GetArgObject(0);
    float min_val = gen->GetArgFloat(1);
    float max_val = gen->GetArgFloat(2);
    float def_val = gen->GetArgFloat(3);
    
    if(this->script_context.float_params.count(*name) == 0)
    {
      this->script_context.float_params[*name] = def_val;
    }
    this->script_events.context_requests.push_back(
      FloatRequest{*name, min_val, max_val, def_val}
    );

    auto curr_val = this->script_context.GetContextRef<float>(*name);
    gen->SetReturnFloat(curr_val);
  });
  as_script_engine->RegisterGlobalFunction("bool Checkbox(string name, bool def_val = true)", [this](asIScriptGeneric *gen)
  {
    std::string *name = (std::string*)gen->GetArgObject(0);
    auto def_val = gen->GetArgByte(1);
    
    if(this->script_context.int_params.count(*name) == 0)
    {
      this->script_context.int_params[*name] = def_val;
    }
    this->script_events.context_requests.push_back(
      BoolRequest{*name, def_val != 0}
    );

    auto curr_val = this->script_context.GetContextRef<int>(*name);
    gen->SetReturnByte(curr_val != 0);
  });

  as_script_engine->RegisterGlobalFunction("void Text(string str)", [this](asIScriptGeneric *gen)
  {
    std::string text = *(std::string*)gen->GetArgObject(0);
    this->script_events.context_requests.push_back(
      TextRequest{text}
    );
  });
  as_script_engine->RegisterGlobalFunction("float GetTime()", [this](asIScriptGeneric *gen)
  {
    gen->SetReturnFloat(this->script_context.curr_time);
  });

  RegisterVecType<ls::vec2, 2>("vec2", "Vec2", "float");
  RegisterVecType<ls::vec3, 3>("vec3", "Vec3", "float");
  RegisterVecType<ls::vec4, 4>("vec4", "Vec4", "float");
  RegisterVecType<ls::ivec2, 2>("ivec2", "IVec2", "int");
  RegisterVecType<ls::ivec3, 3>("ivec3", "IVec3", "int");
  RegisterVecType<ls::ivec4, 4>("ivec4", "IVec4", "int");
  RegisterVecType<ls::ivec2, 2>("uvec2", "UVec2", "uint");
  RegisterVecType<ls::ivec3, 3>("uvec3", "UVec3", "uint");
  RegisterVecType<ls::ivec4, 4>("uvec4", "UVec4", "uint");
  RegisterImageType();
  RegisterBasicTypeOperations();
}

void RenderGraphScript::Impl::RegisterBasicTypeOperations()
{
  as_script_engine->RegisterGlobalFunction("string to_string(float v)", [](asIScriptGeneric *gen)
  {
    float arg = gen->GetArgFloat(0);
    std::string res = std::to_string(arg);
    gen->SetReturnObject(&res);
  });
  as_script_engine->RegisterGlobalFunction("string to_string(int v)", [](asIScriptGeneric *gen)
  {
    int arg = gen->GetArgDWord(0);
    std::string res = std::to_string(arg);
    gen->SetReturnObject(&res);
  });
  as_script_engine->RegisterGlobalFunction("int &ContextInt(string name)", [this](asIScriptGeneric *gen)
  {
    auto *name = (std::string*)gen->GetArgObject(0);
    //this does not invalidate existing points
    gen->SetReturnAddress(&this->script_context.int_params[*name]);
  });
  as_script_engine->RegisterGlobalFunction("float &ContextFloat(string name)", [this](asIScriptGeneric *gen)
  {
    auto *name = (std::string*)gen->GetArgObject(0);
    //this does not invalidate existing points
    gen->SetReturnAddress(&this->script_context.float_params[*name]);
  });  
}

void RenderGraphScript::Impl::RegisterImageType()
{
  as_script_engine->RegisterType<ls::Image>("Image");

  as_script_engine->RegisterGlobalFunction("Image GetMippedImage(int width, int height, PixelFormats pixel_format)", [this](asIScriptGeneric *gen)
  {
    int width = gen->GetArgDWord(0);
    int height = gen->GetArgDWord(1);
    auto pixel_format = ls::PixelFormats(gen->GetArgDWord(2));
    
    Image::Id id = this->image_infos.size();
    this->image_infos.push_back({ivec2{width, height}, pixel_format});

    ls::CachedImageRequest image_request;
    image_request.id = id;
    image_request.pixel_format = pixel_format;
    image_request.size = {width, height};
    this->script_events.context_requests.push_back(image_request);

    ls::Image img;
    img.id = id;
    img.mip_range = ivec2{0, int(this->image_infos[id].GetMipsCount())};
    gen->SetReturnObject(&img);
  });
  as_script_engine->RegisterGlobalFunction("Image GetImage(ivec2 size, PixelFormats pixel_format)", [this](asIScriptGeneric *gen)
  {
    auto size = *(ls::ivec2*)gen->GetArgObject(0);
    auto pixel_format = ls::PixelFormats(gen->GetArgDWord(1));
    
    ls::CachedImageRequest image_request;
    Image::Id id = this->image_infos.size();
    this->image_infos.push_back({size, pixel_format});

    image_request.id = id;
    image_request.pixel_format = pixel_format;
    image_request.size = size;    
    this->script_events.context_requests.push_back(image_request);

    ls::Image img;
    img.id = id;
    img.mip_range = ivec2{0, 1};
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
    if(dst_mip >= src_img.mip_range.y)
    {
      throw ls::RenderGraphRuntimeException(
        0,
        "GetMip",
        std::string("Requested mip ") + std::to_string(mip_level) + " but there are only " + std::to_string(src_img.mip_range.y - src_img.mip_range.x)
      );
    }
    
    int mip = std::clamp<int>(dst_mip, 0, this->image_infos[src_img.id].GetMipsCount() - 1);
    dst_img.mip_range = ivec2{mip, mip + 1};

    gen->SetReturnObject(&dst_img);
  });
  as_script_engine->RegisterMethod("Image", "ivec2 GetSize() const", [this](asIScriptGeneric *gen)
  {
    auto *this_ptr = (ls::Image*)gen->GetObject();
    ivec2 mip_size = this->image_infos[this_ptr->id].GetMipSize(this_ptr->mip_range.x);
    gen->SetReturnObject(&mip_size);
  });
  as_script_engine->RegisterMethod("Image", "int GetMipsCount() const", [this](asIScriptGeneric *gen)
  {
    auto *this_ptr = (ls::Image*)gen->GetObject();
    int mips_count = this_ptr->mip_range.y - this_ptr->mip_range.x;
    gen->SetReturnDWord(mips_count);
  });
  as_script_engine->RegisterMethod("Image", "void Print()", [this](asIScriptGeneric *gen)
  {
    auto *this_ptr = (ls::Image*)gen->GetObject();
    std::cout << "Img id: " << this_ptr->id << " [" << this_ptr->mip_range.x << ", " << this_ptr->mip_range.y << "]\n";
  });
}

template<typename T> T GetArg(asIScriptGeneric *gen, size_t arg_idx) {assert(0);}
template<> float GetArg<float>(asIScriptGeneric *gen, size_t arg_idx){return gen->GetArgFloat(arg_idx);}
template<> int GetArg<int>(asIScriptGeneric *gen, size_t arg_idx){return gen->GetArgDWord(arg_idx);}


template<typename VecType, typename CompType>
CompType GetComp(const VecType &v, size_t idx)
{
  assert(idx * sizeof(CompType) <= sizeof(VecType));
  const CompType *first = &(v.x);
  return first[idx];
}
template<typename VecType, typename CompType>
void SetComp(VecType &v, size_t idx, CompType c)
{
  assert(idx * sizeof(CompType) <= sizeof(VecType));
  CompType *first = &(v.x);
  first[idx] = c;
}

template<typename VecType, size_t CompCount>
void RenderGraphScript::Impl::RegisterVecType(std::string type_name, std::string uppercase_type_name, std::string comp_type_name)
{
  using CompType = decltype(VecType::x);
  assert(sizeof(VecType) == sizeof(CompType) * CompCount);
  as_script_engine->RegisterType<VecType>(type_name.c_str());
  std::string comp_names[] = {"x", "y", "z", "w"};
  std::string constr_decl = "void f(";
  bool is_first = true;
  for(size_t i = 0; i < CompCount; i++)
  {
    if(!is_first)
      constr_decl += ", ";
    is_first = false;
    std::string member_decl = comp_type_name + " " + comp_names[i];
    as_script_engine->RegisterMember(type_name.c_str(), member_decl.c_str(), sizeof(CompType) * i);
    constr_decl += member_decl;
  }
  constr_decl += ")";
  as_script_engine->RegisterConstructor(type_name.c_str(), constr_decl.c_str(), [](asIScriptGeneric *gen)
  {
    auto *this_ptr = (VecType*)gen->GetObject();
    for(size_t i = 0; i < CompCount; i++)
    {
      SetComp(*this_ptr, i, GetArg<CompType>(gen, i));
    }
  });
  as_script_engine->RegisterConstructor(type_name.c_str(), "void f(" + comp_type_name + " v)", [](asIScriptGeneric *gen)
  {
    auto *this_ptr = (VecType*)gen->GetObject();
    for(size_t i = 0; i < CompCount; i++)
    {
      SetComp(*this_ptr, i, GetArg<CompType>(gen, 0));
    }
  });

  as_script_engine->RegisterMethod(type_name.c_str(), (type_name + " " + "opAdd(" + type_name + ") const").c_str(), [=](asIScriptGeneric *gen)
  {
    auto *this_ptr = (VecType*)gen->GetObject();
    auto *other_ptr = (VecType*)gen->GetArgObject(0);
    VecType res;
    for(size_t i = 0; i < CompCount; i++)
    {
      SetComp(res, i, GetComp<VecType, CompType>(*this_ptr, i) + GetComp<VecType, CompType>(*other_ptr, i));
    }
    gen->SetReturnObject(&res);
  });
  as_script_engine->RegisterMethod(type_name.c_str(), "string opAdd_r(string) const", [=](asIScriptGeneric *gen)
  {
    auto *this_ptr = (VecType*)gen->GetObject();
    auto *other_ptr = (std::string*)gen->GetArgObject(0);
    std::string res_str = *other_ptr + "[";
      bool is_first = true;
    for(size_t i = 0; i < CompCount; i++)
    {
      if(!is_first)
        res_str += ", ";
      is_first = false;
      res_str += std::to_string(GetComp<VecType, CompType>(*this_ptr, i));
    }
    res_str += "]";
    gen->SetReturnObject(&res_str);
  });
  as_script_engine->RegisterMethod(type_name.c_str(), (type_name + " " + "opMul(" + type_name + ") const").c_str(), [=](asIScriptGeneric *gen)
  {
    auto *this_ptr = (VecType*)gen->GetObject();
    auto *other_ptr = (VecType*)gen->GetArgObject(0);
    VecType res;
    for(size_t i = 0; i < CompCount; i++)
    {
      SetComp(res, i, GetComp<VecType, CompType>(*this_ptr, i) * GetComp<VecType, CompType>(*other_ptr, i));
    }
    gen->SetReturnObject(&res);
  });
  as_script_engine->RegisterMethod(type_name.c_str(), (type_name + " " + "opMul(" + comp_type_name + ") const").c_str(), [=](asIScriptGeneric *gen)
  {
    auto *this_ptr = (VecType*)gen->GetObject();
    auto other = GetArg<CompType>(gen, 0);
    VecType res;
    for(size_t i = 0; i < CompCount; i++)
    {
      SetComp(res, i, GetComp<VecType, CompType>(*this_ptr, i) * other);
    }
    gen->SetReturnObject(&res);
  });
  as_script_engine->RegisterMethod(type_name.c_str(), (type_name + " " + "opDiv(" + type_name + ") const").c_str(), [=](asIScriptGeneric *gen)
  {
    auto *this_ptr = (VecType*)gen->GetObject();
    auto *other_ptr = (VecType*)gen->GetArgObject(0);
    VecType res;
    for(size_t i = 0; i < CompCount; i++)
    {
      SetComp(res, i, GetComp<VecType, CompType>(*this_ptr, i) / GetComp<VecType, CompType>(*other_ptr, i));
    }
    gen->SetReturnObject(&res);
  });
  as_script_engine->RegisterMethod(type_name.c_str(), (type_name + " " + "opDiv(" + comp_type_name + ") const").c_str(), [=](asIScriptGeneric *gen)
  {
    auto *this_ptr = (VecType*)gen->GetObject();
    auto other = GetArg<CompType>(gen, 0);
    VecType res;
    for(size_t i = 0; i < CompCount; i++)
    {
      SetComp(res, i, GetComp<VecType, CompType>(*this_ptr, i) / other);
    }
    gen->SetReturnObject(&res);
  });

  as_script_engine->RegisterGlobalFunction(std::string("string to_string(") + type_name + " v)", [](asIScriptGeneric *gen)
  {
    auto *arg_ptr = (VecType*)gen->GetArgObject(0);

    std::string res = "[";
    bool is_first = true;
    for(size_t i = 0; i < CompCount; i++)
    {
      if(!is_first) res += ", ";
      is_first = false;
      res += std::to_string(GetComp<VecType, CompType>(*arg_ptr, i));
    }
    res += "]";
    gen->SetReturnObject(&res);
  });
  as_script_engine->RegisterGlobalFunction(type_name + "& Context" + uppercase_type_name + "(string name)", [this](asIScriptGeneric *gen)
  {
    auto *name_ptr = (std::string*)gen->GetArgObject(0);
    gen->SetReturnObject(&this->script_context.GetContextRef<VecType>(*name_ptr));
  });
}


void RenderGraphScript::Impl::RegisterAsScriptPassFunctions(const std::vector<ls::PassDecl> &pass_decls)
{
  for(const auto &pass_decl : pass_decls)
  {
    std::string as_func_decl = CreateAsPassFuncDeclaration(pass_decl);
    this->as_script_engine->RegisterGlobalFunction(as_func_decl, [this, pass_decl](asIScriptGeneric *gen)
    {
      ShaderInvocation invocation;
      invocation.shader_name = pass_decl.name;
      for(size_t param_idx = 0; param_idx < pass_decl.arg_descs.size(); param_idx++)
      {
        AddScriptInvocationAsArg(invocation, gen, param_idx, pass_decl.arg_descs[param_idx]);
      }
      this->script_events.script_shader_invocations.push_back(invocation);
    });
  }
}

}