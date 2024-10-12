#include "ScriptParser.h"
#include <iostream>
#include <peglib.h>
namespace ls
{
  struct int3
  {
    int x, y, z; 
  };

  
  struct ScriptParser::Impl
  {
    static std::string GetScriptGrammar()
    {
      return R"(
        # Grammar for function parsing
        Script              <- Block*
        Block               <- Preamble BlockDecl '{{' BlockBody '}}'
        BlockDecl           <- (RenderGraphDecl / PassDecl)?
        Preamble            <- (PreambleSection)*
        PreambleSection     <- '[' (DeclarationSection / IncludeSection / NumthreadsSection) ']'
        DeclarationSection  <- 'declaration' ':' String
        IncludeSection      <- 'include' ':' StringArray
        NumthreadsSection   <- 'numthreads' Int3
        RenderGraphDecl     <- 'void' 'RenderGraphMain' '(' ArgDescs ')'
        PassDecl            <- PodType Name '(' ArgDescs ')'
        BlockBody           <- (!('}}') .)*
        ArgDescs            <- ArgDesc? (',' ArgDesc)* 
        ArgDesc             <- (DecoratedPodType / DecoratedImageType / SamplerType) Name
        PodType             <- <'void' | 'float' | 'vec2' | 'vec3' | 'vec4' | 'int' | 'ivec2' | 'ivec3' | 'ivec4'>
        DecoratedPodType    <- PodAccess? PodType
        PodAccess           <- <'in' | 'out' | 'inout'>
        DecoratedImageType  <- ImageAccess? ImageType '<' ImagePixelFormat '>'
        ImageAccess         <- <'readonly' | 'writeonly' | 'readwrite'>
        ImageType           <- <'image1D' | 'image2D' | 'image3D'>
        ImagePixelFormat    <- <'rgba8' | 'rgba16f' | 'rgba32f'>
        SamplerType         <- <'sampler1D' | 'sampler2D' | 'sampler3D'>
        Name                <- < [a-zA-Z_][a-zA-Z_0-9]* >
        StringArray         <- String (',' String)*
        String              <- < '\"' UnquotedString '\"' >
        UnquotedString      <- (!('\"') .)*
        Int3                <- '(' Int ',' Int ',' Int ')'
        Int                 <- < [0-9][0-9]* >
        %whitespace         <- [ \t\r\n]*
        %word               <- Name
      )";
    }
    Impl()
    {
      parser.set_logger([](size_t line, size_t col, const std::string& msg, const std::string &rule) {
        std::cerr << line << ":" << col << ": " << msg << "\n";
      });
      
      parser.load_grammar(GetScriptGrammar());

      parser["Script"] = [](const peg::SemanticValues &vs) -> ParsedScript
      {
        ParsedScript script;
        for(const auto &v : vs)
        {
          script.blocks.push_back(std::any_cast<Block>(v));
        }
        return script;
      };
      
      parser["Block"] = [](const peg::SemanticValues &vs) -> Block
      {
        Block block;
        block.preamble = std::any_cast<Preamble>(vs[0]);
        block.decl = vs[1].has_value() ? std::any_cast<BlockDecl>(vs[1]) : BlockDecl();
        block.body = std::any_cast<std::string>(vs[2]);
        return block;
      };

      parser["Preamble"] = [](const peg::SemanticValues &vs) -> Preamble
      {
        Preamble preamble;
        for(auto &v : vs)
        {
          preamble.push_back(std::any_cast<PreambleSection>(v));
        }
        return preamble;
      };

      parser["DeclarationSection"] = [](const peg::SemanticValues &vs) -> PreambleSection
      {
        DeclarationSection section;
        section.name = std::any_cast<std::string>(vs[0]);
        return section;
      };

      parser["IncludeSection"] = [](const peg::SemanticValues &vs) -> PreambleSection
      {
        IncludeSection section;
        section.include_names = std::any_cast<StringArray>(vs[0]);
        return section;
      };

      parser["NumthreadsSection"] = [](const peg::SemanticValues &vs) -> PreambleSection
      {
        NumthreadsSection section;
        int3 vec = std::any_cast<int3>(vs[0]);
        section.x = vec.x;
        section.y = vec.y;
        section.z = vec.z;
        return section;
      };

      parser["RenderGraphDecl"] = [](const peg::SemanticValues &vs) -> BlockDecl
      {
        RenderGraphDecl graph_decl;
        graph_decl.arg_descs = std::any_cast<ArgDescs>(vs[0]);
        return graph_decl;
      };
      
      parser["PassDecl"] = [](const peg::SemanticValues &vs) -> BlockDecl
      {
        PassDecl pass_decl;
        pass_decl.return_type = std::any_cast<DecoratedPodType::PodTypes>(vs[0]);
        pass_decl.name = std::any_cast<std::string>(vs[1]);
        pass_decl.arg_descs = std::any_cast<ArgDescs>(vs[2]);
        return pass_decl;
      };

      
      parser["BlockBody"] = [](const peg::SemanticValues &vs) -> std::string
      {
        return vs.token_to_string();
      };

      parser["ArgDescs"] = [](const peg::SemanticValues &vs) -> ArgDescs
      {
        ArgDescs arg_descs;
        for(const auto &v : vs)
        {
          arg_descs.push_back(std::any_cast<ArgDesc>(v));
        }
        return arg_descs;
      };
      
      parser["ArgDesc"] = [](const peg::SemanticValues &vs) -> ArgDesc
      {
        ArgDesc arg_desc;
        arg_desc.type = std::any_cast<ArgDesc::ArgType>(vs[0]);
        arg_desc.name = std::any_cast<std::string>(vs[1]);
        return arg_desc;
      };
      
      parser["PodType"] = [](const peg::SemanticValues &vs) -> DecoratedPodType::PodTypes
      {
        return DecoratedPodType::PodTypes(vs.choice());
      };

      parser["DecoratedPodType"] = [](const peg::SemanticValues &vs) -> ArgDesc::ArgType
      {
        DecoratedPodType dec_pod_type;
        size_t used_count = 0;
        if(vs.size() == 2)
          dec_pod_type.access_qalifier = std::any_cast<DecoratedPodType::AccessQualifiers>(vs[used_count++]);
        dec_pod_type.type = std::any_cast<DecoratedPodType::PodTypes>(vs[used_count++]);
        return ArgDesc::ArgType(dec_pod_type);
      };
      parser["PodAccess"] = [](const peg::SemanticValues &vs) -> DecoratedPodType::AccessQualifiers
      {
        return DecoratedPodType::AccessQualifiers(vs.choice());
      };
      
      
      parser["ImageType"] = [](const peg::SemanticValues &vs) -> ImageTypes
      {
        return ImageTypes(vs.choice());
      };
      parser["ImagePixelFormat"] = [](const peg::SemanticValues &vs) -> PixelFormats
      {
        return PixelFormats(vs.choice());
      };
      parser["ImageAccess"] = [](const peg::SemanticValues &vs) -> AccessQualifiers
      {
        return AccessQualifiers(vs.choice());
      };

      parser["DecoratedImageType"] = [](const peg::SemanticValues &vs) -> ArgDesc::ArgType
      {
        DecoratedImageType dec_img_type;
        size_t used_count = 0;    
        if(vs.size() == 3)
          dec_img_type.access_qualifiers = std::any_cast<AccessQualifiers>(vs[used_count++]);
        dec_img_type.image_type = std::any_cast<ImageTypes>(vs[used_count++]);
        dec_img_type.pixel_format = std::any_cast<PixelFormats>(vs[used_count++]);
        return ArgDesc::ArgType(dec_img_type);
      };
      parser["SamplerType"] = [](const peg::SemanticValues &vs) -> ArgDesc::ArgType
      {
        return ArgDesc::ArgType(SamplerTypes(vs.choice()));
      };

      
      parser["Name"] = [](const peg::SemanticValues &vs) -> std::string
      {
        return vs.token_to_string();
      };

      parser["StringArray"] = [](const peg::SemanticValues &vs) -> StringArray
      {
        StringArray string_array;
        for(const auto v : vs)
        {
          string_array.push_back(std::any_cast<std::string>(v));
        }
        return string_array;
      };

      parser["UnquotedString"] = [](const peg::SemanticValues &vs) -> std::string
      {
        return vs.token_to_string();
      };

      parser["Int3"] = [](const peg::SemanticValues &vs) -> int3
      {
        int3 vec;
        vec.x = std::any_cast<int>(vs[0]);
        vec.y = std::any_cast<int>(vs[1]);
        vec.z = std::any_cast<int>(vs[2]);
        return vec;
      };

      parser["Int"] = [](const peg::SemanticValues &vs) -> int
      {
        return vs.token_to_number<int>();
      };
    }
    peg::parser parser;
  };
  
  ScriptParser::ScriptParser()
  {
    impl.reset(new Impl());
  }
  ScriptParser::~ScriptParser()
  {
  }
  
  ParsedScript ScriptParser::Parse(std::string script_src)
  {
    ParsedScript script;
    bool res = impl->parser.parse(script_src, script);
    if(!res)
      throw std::runtime_error("Failed to parse the script");
    return script;
  }
  std::string PodTypeToString(ls::DecoratedPodType::PodTypes type)
  {
    using PodTypes = ls::DecoratedPodType::PodTypes;
    switch(type)
    {
      case PodTypes::float_: return "float"; break;
      case PodTypes::vec2: return "vec2"; break;
      case PodTypes::vec3: return "vec3"; break;
      case PodTypes::vec4: return "vec4"; break;
      case PodTypes::int_: return "int"; break;
      case PodTypes::void_: return "void"; break;
      default: throw std::runtime_error("Can't generate as type");
    }
  }

}