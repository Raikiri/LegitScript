#include "../include/LegitScript.h"
#include "ScriptParser.h"
#include "RenderGraphScript.h"

namespace ls
{
  ls::ScriptShaderDesc CreateShaderDesc(const ls::PassDecl &decl, const ls::Preamble &preamble, std::string body)
  {
    ls::ScriptShaderDesc desc;
    desc.body = body;
    desc.name = decl.name;
    desc.includes = "";
    
    for(const auto &arg : decl.arg_descs)
    {
      if(std::holds_alternative<ls::DecoratedPodType>(arg.type))
      {
        auto dec_pod_type = std::get<ls::DecoratedPodType>(arg.type);
        std::string type_name = PodTypeToString(dec_pod_type.type);
        
        auto access_qualifier = dec_pod_type.access_qalifier.value_or(ls::DecoratedPodType::AccessQualifiers::in);
        if(access_qualifier == ls::DecoratedPodType::AccessQualifiers::out)
        {
          desc.outs.push_back({type_name, arg.name});
        }else
        {
          desc.uniforms.push_back({type_name, arg.name});
        }
      }
    }
    return desc;
  }
  
  struct LegitScript::Impl
  {
    Impl(SliderFloatFunc slider_float_func, SliderIntFunc slider_int_func, TextFunc text_func)
      : render_graph_script(slider_float_func, slider_int_func, text_func)
    {
    }
    ~Impl(){}
    ls::ScriptShaderDescs LoadScript(std::string script_source)
    {
      ls::ScriptShaderDescs shader_descs;
      auto parsed_script = script_parser.Parse(script_source);
      std::vector<PassDecl> pass_decls;
      for(const auto &block : parsed_script.blocks)
      {
        if(block.decl.has_value() && std::holds_alternative<ls::PassDecl>(block.decl.value()))
        {
          auto pass_decl = std::get<ls::PassDecl>(block.decl.value());
          pass_decls.push_back(pass_decl);
          
          shader_descs.push_back(CreateShaderDesc(pass_decl, block.preamble, block.body));
        }
      }
      
      for(const auto &block : parsed_script.blocks)
      {
        if(block.decl.has_value() && std::holds_alternative<ls::RenderGraphDecl>(block.decl.value()))
        {
          auto render_graph_decl = std::get<ls::RenderGraphDecl>(block.decl.value());
          render_graph_script.LoadScript(block.body, pass_decls);
        }
      }

      return shader_descs;
    }
    ls::ScriptEvents RunScript(ivec2 swapchain_size, float time)
    {
      return render_graph_script.RunScript(swapchain_size, time);
    }
  private:
    ls::RenderGraphScript render_graph_script;
    ls::ScriptParser script_parser;
  };
  
  ls::ScriptEvents LegitScript::RunScript(ivec2 swapchain_size, float time)
  {
    return impl->RunScript(swapchain_size, time);
  }

  ls::ScriptShaderDescs LegitScript::LoadScript(std::string script_source)
  {
    return impl->LoadScript(script_source);
  }
  
  LegitScript::LegitScript(SliderFloatFunc slider_float_func, SliderIntFunc slider_int_func, TextFunc text_func)
  {
    this->impl.reset(new LegitScript::Impl(slider_float_func, slider_int_func, text_func));
  }
  LegitScript::~LegitScript()
  {
  }
}
