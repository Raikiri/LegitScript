#pragma once
#include <functional>
#include <string>

namespace ls
{
  struct LoadedImage
  {
    ivec2 size;
    ls::PixelFormats pixel_format;
  };
  using ContextValueType = std::variant<float, vec2, vec3, vec4, int, ivec2, ivec3, ivec4, LoadedImage>;
  struct ContextInput
  {
    std::string name;
    ContextValueType value;
  };
  //using ContextInputs = std::vector<ContextInput>;
}
