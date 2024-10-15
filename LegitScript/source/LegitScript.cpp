#include "../include/LegitScript.h"
#include "ScriptParser.h"
#include "RenderGraphScript.h"
#include <assert.h>

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
      }else
      if(std::holds_alternative<ls::SamplerTypes>(arg.type))
      {
        auto sampler_type = std::get<ls::SamplerTypes>(arg.type);
        desc.samplers.push_back({SamplerTypeToString(sampler_type), arg.name});
      }else
      {
        assert(0);
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
      try
      {
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
            try
            {
              this->render_graph_block_start = block.body_start;
              render_graph_script.LoadScript(block.body, pass_decls);
            }
            catch(const ls::RenderGraphBuildException &e)
            {
              throw ls::ScriptException(
                e.line + render_graph_block_start - 1, //-1 because line 1 corresponds to render_graph_block_start
                e.column,
                "",
                e.desc
              );
            }
          }
        }

        return shader_descs;
      }catch(const ls::ScriptParserException &e)
      {
        throw ls::ScriptException(
          e.line,
          e.column,
          "",
          e.desc
        );
      }
    }
    ls::ScriptEvents RunScript(ivec2 swapchain_size, float time)
    {
      try
      {
        return render_graph_script.RunScript(swapchain_size, time);
      }
      catch(const ls::RenderGraphRuntimeException &e)
      {
        throw ls::ScriptException(
          e.line + this->render_graph_block_start - 1, //-1 because line 1 corresponds to render_graph_block_start
          0,
          e.func,
          e.desc
        );
      }
    }
  private:
    ls::RenderGraphScript render_graph_script;
    size_t render_graph_block_start = 0;
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
