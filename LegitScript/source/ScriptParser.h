#pragma once
#include "../include/LegitScriptEvents.h"
#include <vector>
#include <string>
#include <variant>
#include <optional>
#include <memory>
#include <stdexcept>

namespace ls
{
  struct DecoratedPodType
  {
    enum struct PodTypes : int
    {
      void_,
      float_,
      vec2,
      vec3,
      vec4,
      int_,
      ivec2,
      ivec3,
      ivec4,
      uint_,
      uvec2,
      uvec3,
      uvec4,
      undefined
    };
    PodTypes type;
    
    enum struct AccessQualifiers : int
    {
      in,
      out
    };
    using OptionalAccessQualifiers = std::optional<AccessQualifiers>;
    OptionalAccessQualifiers access_qalifier;
  };


  enum struct ImageTypes : int
  {
    image1D,
    image2D,
    image3D
  };
  enum struct AccessQualifiers : int
  {
    readonly,
    writeonly,
    readwrite
  };

  struct DecoratedImageType
  {
    ImageTypes image_type;
    PixelFormats pixel_format;
    using OptionalAccessQualifiers = std::optional<AccessQualifiers>;
    OptionalAccessQualifiers access_qualifiers;
  };

  struct DeclarationSection
  {
    std::string name;
  };

  using StringArray = std::vector<std::string>;
  struct IncludeSection
  {
    StringArray include_names;
  };

  struct NumthreadsSection
  {
    int x, y, z;
  };

  struct RendergraphSection
  {
  };
  
  using PreambleSection = std::variant<RendergraphSection, BlendModes, DeclarationSection, IncludeSection, NumthreadsSection>;
  struct ArgDesc
  {
    using ArgType = std::variant<DecoratedPodType, DecoratedImageType, SamplerTypes>;
    ArgType type;
    std::string name;
  };
  using ArgDescs = std::vector<ArgDesc>;

  struct PassDecl
  {
    enum struct Types
    {
      FullscreenPass
    };
    Types type = Types::FullscreenPass;
    DecoratedPodType::PodTypes return_type = DecoratedPodType::PodTypes::undefined;
    std::string name;
    ArgDescs arg_descs;
  };
  
  using Preamble = std::vector<PreambleSection>;

  using BlockDecl = std::optional<PassDecl>;
  struct Block
  {
    BlockDecl decl;
    Preamble preamble;
    BlockBody body;
  };

  struct ParsedScript
  {
    std::vector<Block> blocks;
  };
  
  std::string PodTypeToString(ls::DecoratedPodType::PodTypes type);
  std::string SamplerTypeToString(ls::SamplerTypes type);
  
  class ScriptParserException : public std::runtime_error
  {
  public:
    ScriptParserException(
      size_t line,
      size_t column,
      std::string desc) :
      line(line),
      column(column),
      desc(desc),
      std::runtime_error(std::string("[") + std::to_string(line) + ":" + std::to_string(column) + "]" + ":" + desc){}

    size_t line;
    size_t column;
    std::string desc;
  };
  struct ScriptParser
  {
    ScriptParser();
    ~ScriptParser();
    ParsedScript Parse(std::string script_src);
  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
  };
}