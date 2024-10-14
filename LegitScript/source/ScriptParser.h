#pragma once
#include "../include/LegitScriptEvents.h"
#include <vector>
#include <string>
#include <variant>
#include <optional>
#include <memory>

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

  using PreambleSection = std::variant<DeclarationSection, IncludeSection, NumthreadsSection>;
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
  
  struct RenderGraphDecl
  {
    ArgDescs arg_descs;
  };

  using Preamble = std::vector<PreambleSection>;
  
  using BlockDecl = std::optional<std::variant<PassDecl, RenderGraphDecl>>;
  struct Block
  {
    BlockDecl decl;
    Preamble preamble;
    std::string body;
  };

  struct ParsedScript
  {
    std::vector<Block> blocks;
  };
  
  std::string PodTypeToString(ls::DecoratedPodType::PodTypes type);
  std::string SamplerTypeToString(ls::SamplerTypes type);
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