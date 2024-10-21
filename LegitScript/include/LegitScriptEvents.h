#pragma once
#include <vector>
#include <string>
#include <functional>
#include <variant>
#include <cstdint>
#include "PodTypes.h"

namespace ls
{
  enum struct SamplerTypes : int
  {
    sampler1D,
    sampler2D,
    sampler3D
  };
  
  enum struct PixelFormats : int
  {
    rgba8,
    rgba16f,
    rgba32f
  };

  struct Image
  {
    using Id = size_t;
    Id id;
    ivec2 mip_range;
  };
  
  enum struct BlendModes : int
  {
    opaque,
    alphablend,
    additive,
    multiplicative
  };

  struct BlockBody
  {
    std::string text;
    size_t start;
  };

  struct ShaderDesc
  {
    struct Sampler
    {
      std::string type;
      std::string name;
    };
    std::vector<Sampler> samplers;
    struct Uniform
    {
      std::string type;
      std::string name;
    };
    std::vector<Uniform> uniforms;
    struct Image
    {
      std::string type;
      std::string name;
    };
    std::vector<Image> images;
    
    struct Inouts
    {
      std::string type;
      std::string name;
    };
    std::vector<Inouts> ins;
    std::vector<Inouts> outs;
    
    BlendModes blend_mode;
    std::string name;
    
    std::vector<std::string> includes;
    BlockBody body;
  };
  using ShaderDescs = std::vector<ShaderDesc>;
  struct Declaration
  {
    std::string name;
    BlockBody body;
  };
  using Declarations = std::vector<Declaration>;
  struct ScriptContents
  {
    ShaderDescs shader_descs;
    Declarations declarations;
  };

  struct ShaderInvocation
  {
    std::string shader_name;
    std::vector<ls::Image> image_sampler_bindings;
    std::vector<ls::Image> color_attachments;

    struct UniformValue
    {
      size_t offset;
      size_t size;
    };
    template<typename T>
    void AddUniformValue(T t)
    {
      UniformValue new_value;
      new_value.offset = uniform_data.size();
      new_value.size = sizeof(T);
      const auto *ptr = (uint8_t*)&t;
      size_t size = sizeof(t);
      for(size_t byte_idx = 0; byte_idx < size; byte_idx++)
        uniform_data.push_back(ptr[byte_idx]);
      uniform_values.push_back(new_value);
    }
    std::vector<UniformValue> uniform_values;
    std::vector<uint8_t> uniform_data;
  };

  struct CachedImageRequest
  {
    ls::PixelFormats pixel_format;
    ivec2 size;
    Image::Id id;
    bool operator < (const CachedImageRequest &other) const
    {
      return std::tie(pixel_format, size.x, size.y, id) < std::tie(other.pixel_format, other.size.x, other.size.y, other.id);
    }
  };

  struct LoadedImageRequest
  {
    std::string filename;
    Image::Id id;
  };
  

  struct FloatRequest
  {
    std::string name;
    float min_val, max_val, def_val;
  };
  struct IntRequest
  {
    std::string name;
    int min_val, max_val, def_val;
  };
  struct ColorRequest
  {
    std::string name;
    vec4 def_val;
  };
  struct BoolRequest
  {
    std::string name;
    bool def_val;
  };
  struct TextRequest
  {
    std::string text;
  };
  using ContextRequest = std::variant<FloatRequest, IntRequest, ColorRequest, BoolRequest, TextRequest, LoadedImageRequest, CachedImageRequest>;
  
  struct ScriptEvents
  {
    std::vector<ContextRequest> context_requests;
    std::vector<ShaderInvocation> script_shader_invocations;
  };
}