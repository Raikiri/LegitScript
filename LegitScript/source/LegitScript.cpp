#include "../include/LegitScript.h"
#include "ScriptParser.h"
#include "RenderGraphScript.h"
#include <assert.h>
#include <map>
#include "IncludeGraph.h"
#include "../include/SourceAssembler.h"
namespace ls
{
  ls::ShaderDesc CreateShaderDesc(const ls::PassDecl &decl, std::vector<std::string> flattened_includes, const ls::Preamble &preamble, ls::BlockBody body)
  {
    ls::ShaderDesc desc;
    desc.body = body;
    desc.name = decl.name;
    desc.includes = flattened_includes;
    desc.blend_mode = BlendModes::opaque;
    for(const auto &p : preamble)
    {
      if(std::holds_alternative<ls::BlendModes>(p))
      {
        desc.blend_mode = std::get<ls::BlendModes>(p);
      }
    }
    
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
  std::optional<std::string> FindPreambleDeclName(const ls::Preamble &preamble)
  {
    for(auto p : preamble)
    {
      if(std::holds_alternative<ls::DeclarationSection>(p))
      {
        auto decl = std::get<ls::DeclarationSection>(p);
        return decl.name;
      }
    }
    return std::nullopt;
  }
  std::vector<std::string> FindPreambleIncludes(const ls::Preamble &preamble)
  {
    std::vector<std::string> includes;
    for(auto p : preamble)
    {
      if(std::holds_alternative<ls::IncludeSection>(p))
      {
        auto incl = std::get<ls::IncludeSection>(p);
        for(auto name : incl.include_names)
        {
          includes.push_back(name);
        }
      }
    }
    return includes;
  }
  
  bool FindPreambleIsRendergraph(const ls::Preamble &preamble)
  {
    for(auto p : preamble)
    {
      if(std::holds_alternative<ls::RendergraphSection>(p))
      {
        return true;
      }
    }
    return false;
  }
  
  ls::Graph BuildBlockDirectGraph(const std::vector<ls::Block> &blocks)
  {
    std::map<std::string, size_t> name_to_idx;
    for(size_t block_idx = 0; block_idx < blocks.size(); block_idx++)
    {
      const auto &block = blocks[block_idx];
      auto opt_name = FindPreambleDeclName(block.preamble);
      if(opt_name)
        name_to_idx[opt_name.value()] = block_idx;  
    }
    ls::Graph graph;
    for(size_t block_idx = 0; block_idx < blocks.size(); block_idx++)
    {
      const auto &block = blocks[block_idx];
      ls::GraphNode node;
      auto includes = FindPreambleIncludes(block.preamble);
      for(auto name : includes)
      {
        auto name_it = name_to_idx.find(name);
        if(name_it == name_to_idx.end())
          throw ls::ScriptException(block.body.start, 0, "", std::string("Included block ") + name + " does not exist");
        node.adjacent_nodes.push_back(name_it->second);
      }
      graph.push_back(node);
    }
    return graph;
  }

  struct LegitScript::Impl
  {
    Impl(SliderFloatFunc slider_float_func, SliderIntFunc slider_int_func, TextFunc text_func)
      : render_graph_script(slider_float_func, slider_int_func, text_func)
    {
    }
    ~Impl(){}
    ls::ScriptContents LoadScript(std::string script_source)
    {
      ls::ScriptContents script_contents;
      ls::ParsedScript parsed_script;
      try
      {
        parsed_script = script_parser.Parse(script_source);
      }catch(const ls::ScriptParserException &e)
      {
        throw ls::ScriptException(
          e.line,
          e.column,
          "",
          e.desc
        );
      }
      auto direct_include_graph = BuildBlockDirectGraph(parsed_script.blocks);
      auto flattened_include_graph = ls::FlattenGraph(direct_include_graph);

      std::vector<PassDecl> pass_decls;
      
      for(size_t block_idx = 0; block_idx < parsed_script.blocks.size(); block_idx++)
      {
        const auto &block = parsed_script.blocks[block_idx];
        if(!FindPreambleIsRendergraph(block.preamble))
        {
          if(block.decl.has_value())
          {
            auto pass_decl = block.decl.value();
            pass_decls.push_back(pass_decl);
            std::vector<std::string> includes;
            for(auto included_idx : flattened_include_graph[block_idx].adjacent_nodes)
            {
              auto opt_name = FindPreambleDeclName(parsed_script.blocks[included_idx].preamble);
              assert(opt_name);
              includes.push_back(opt_name.value());
            }
            script_contents.shader_descs.push_back(CreateShaderDesc(pass_decl, includes, block.preamble, block.body));
          }else
          {
            auto opt_name = FindPreambleDeclName(parsed_script.blocks[block_idx].preamble);
            if(opt_name)
            {
              ls::Declaration decl;
              decl.body = block.body;
              decl.name = opt_name.value();
              script_contents.declarations.push_back(decl);
            }
          }
        }
      }
      
      for(size_t block_idx = 0; block_idx < parsed_script.blocks.size(); block_idx++)
      {
        const auto &block = parsed_script.blocks[block_idx];
        if(FindPreambleIsRendergraph(block.preamble))
        {
          if(!block.decl.has_value())
          {
            throw ls::ScriptException(
              0, 0, "", "Render graph block has to have a declaration"
            );
          }
          source_assembler.reset(new ls::SourceAssembler());
          for(auto included_idx : flattened_include_graph[block_idx].adjacent_nodes)
          {
            const auto &included_body = parsed_script.blocks[included_idx].body;
            source_assembler->AddSourceBlock(included_body.text, included_body.start);
          }
          source_assembler->AddSourceBlock(block.body.text, block.body.start);
          try
          {
            render_graph_script.LoadScript(source_assembler->GetSource(), pass_decls);
          }
          catch(const ls::RenderGraphBuildException &e)
          {
            auto opt_line = source_assembler->GetSourceLine(e.line);
            throw ls::ScriptException(
              opt_line ? opt_line.value() : 0,
              e.column,
              "",
              e.desc
            );
          }
        }
      }

      return script_contents;
    }
    ls::ScriptEvents RunScript(ivec2 swapchain_size, float time)
    {
      try
      {
        return render_graph_script.RunScript(swapchain_size, time);
      }
      catch(const ls::RenderGraphRuntimeException &e)
      {
        auto opt_line = source_assembler->GetSourceLine(e.line);
        throw ls::ScriptException(
          opt_line ? opt_line.value() : 0,
          0,
          e.func,
          e.desc
        );
      }
    }
  private:
    ls::RenderGraphScript render_graph_script;
    std::unique_ptr<ls::SourceAssembler> source_assembler;
    ls::ScriptParser script_parser;
  };
  
  ls::ScriptEvents LegitScript::RunScript(ivec2 swapchain_size, float time)
  {
    return impl->RunScript(swapchain_size, time);
  }

  ls::ScriptContents LegitScript::LoadScript(std::string script_source)
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
