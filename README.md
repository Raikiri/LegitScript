# LegitScript

![multiplatform build](https://github.com/Raikiri/LegitScript/actions/workflows/cmake-multi-platform.yml/badge.svg) ![emscripten build](https://github.com/Raikiri/LegitScript/actions/workflows/emscripten.yml/badge.svg)

LegitScript is a crossplatform glsl-like hybrid CPU/GPU scripting language that has the goal of defining both the shaders and also the way those shaders need to be invoked. The goal is to make the entire language look like it's an extension over glsl.

```cpp
void ColorPass(in float r, in float g, in float b, out vec4 out_color)
{{
  // Inside of this block is valid glsl code
  void main()
  {
    out_color = vec4(r, g, b + 0.5f, 1.0f);
  }
}}

void RenderGraphMain()
{{
  // Inside of this block is valid angelscript code that's meant to look like glsl
  void main()
  {
    ColorPass(
      SliderFloat("R", 0.0f, 1.0f),
      SliderFloat("G", 0.0f, 1.0f),
      SliderFloat("B", 0.0f, 1.0f),
      GetSwapchainImage());
    int a = SliderInt("Int param", -42, 42, 5);
    float b = SliderFloat("Float param", -42.0f, 42.0f);
    Text("script int: " + formatInt(a) + " float: " + formatFloat(b));
  }
}}
```

# How is it meant to be used?
Shaders themselves are written in conventional glsl, except their bindings (uniforms, images, samplers) are generated automatically by LegitScript.
Every shader is declared as a function where every `in` parameter is a uniform/image and every `out` parameter is a render target. Calling this function from the render graph invokes the shader with whatever arguments out pass into it:
uniforms are automatically accessible from a constant buffer, images are automatically bound into samplers and a render target is bound for every out parameter. In the example above `void RenderGraphMain()` is the block defining a render
graph. `void main()` in that block is the render graph entry function: it invokes the shader called `ColorPass()`, passing `r, g, b` uniforms directly from debug sliders and the swapchain is bound as its render target.

This library is a middleware that does not run or compile shaders themselves, but instead it runs the script and outputs a list of shader invocations that happened during that script: which shaders were called with which parameters. The idea is that
multiple GPU backends (for example, Vulkan or webgl) can easily parse that list and run the shaders. Here's a minimal example of how the library functions:
```cpp
void RunTest()
{
  std::cout << "Test starts\n";  
  ls::LegitScript script(
    [](std::string name, float val, float min_val, float max_val) -> float {
      std::cout << "Slider int: " << val << "[" << min_val << ", " << max_val << "]\n";
      return val;
    },
    [](std::string name, int val, int min_val, int max_val) -> int {
      std::cout << "Slider float: " << val << "[" << min_val << ", " << max_val << "]\n";
      return val;
    },
    [](std::string text) -> void {
      std::cout << "Text: " << text << "\n";
    }
  );
  
  std::ifstream file_stream("../data/Scripts/main.ls");
  std::stringstream string_stream;
  string_stream << file_stream.rdbuf();
  std::cout << "Loading script\n";
  try
  {
    auto shader_descs = script.LoadScript(string_stream.str());
    for(const auto &shader_desc : shader_descs)
      PrintShaderDesc(shader_desc);
      
    auto script_calls = script.RunScript({1024, 1024}, 0.0f);
    for(const auto &req : script_calls.cached_image_requests)
      PrintCachedImgRequest(req);
    for(const auto &inv : script_calls.script_shader_invocations)
      PrintShaderInvocation(inv);
  }
  catch(const std::exception &e)
  {
    std::cout << "Exception: " << e.what();
  }
}
```

# How does it actually work?
The script is split into blocks defined inside double curly brackets: `{{}}`. Blocks corresponding to shader passes are appended to glsl shader headers and can be directly compiled as glsl. For each such a block, LegitScript returns
one generated glsl shader ready to be compiled. Block named `void RenderGraphMain()` is the render graph function and it's internally compiled by LegitScript as AngelScript. AngelScript is chosen as the closes to glsl language that can be interpreted easily from C++.
`RunScript()` is meant to be called every frame and it outputs all events that happen during that frame: loading images, running shaders, etc. This information is meant to be easily translateable into actual draw calls on any GAPI backend
that supports glsl.

# String-only JSON interface
For the purposes of embedding LegitScript into web, we support an emscripten build and a dedicated string-only interface for easy integration with JavaScript code:
```cpp
  std::cout << "Test starts\n";  
  ls::InitScript([](std::string name, float val, float min_val, float max_val) -> float
    {
      std::cout << "Slider int: " << val << "[" << min_val << ", " << max_val << "]\n";
      return val;
    },
    [](std::string name, int val, int min_val, int max_val) -> int
    {
      std::cout << "Slider float: " << val << "[" << min_val << ", " << max_val << "]\n";
      return val;
    },
    [](std::string text) -> void
    {
      std::cout << "Text: " << text << "\n";
    });

  std::ifstream file_stream("../data/Scripts/main.ls");
  std::stringstream string_stream;
  string_stream << file_stream.rdbuf();
  try
  {
    std::string shader_descs = ls::LoadScript(string_stream.str());      
    std::string script_calls = ls::RunScript(1024, 1024, 0.0f);
  }
  catch(const std::exception &e)
  {
    std::cout << "Exception: " << e.what();
  }
```
Shader descs output:
```json
{
  "shader_descs": [
    {
      "body": "void main()\n  {\n    out_color = vec4(r, g, b + 0.5f, 1.0f);\n  }\n",
      "name": "ColorPass",
      "outs": [
        {
          "name": "out_color",
          "type": "vec4"
        }
      ],
      "samplers": [],
      "uniforms": [
        {
          "name": "r",
          "type": "float"
        },
        {
          "name": "g",
          "type": "float"
        },
        {
          "name": "b",
          "type": "float"
        }
      ]
    }
  ]
}
```
Script calls output:
```json
{
  "cached_img_requests": [],
  "loaded_img_requests": [],
  "shader_invocations": [
    {
      "color_attachments": [
        {
          "id": 0,
          "mip_end": 1,
          "mip_start": 0
        }
      ],
      "image_sampler_bindings": [],
      "shader_name": "ColorPass",
      "uniforms": [
        {
          "type": "float",
          "val": 0.0
        },
        {
          "type": "float",
          "val": 0.0
        },
        {
          "type": "float",
          "val": 0.0
        }
      ]
    }
  ]
}
```
# Dependencies (bundled in)
`cpp-peglib` is used to parse LegitScript syntax: https://github.com/yhirose/cpp-peglib

`angelscript` is used as the interpreter of the render graph code: https://www.angelcode.com/angelscript/

# Building and intended usage
LegitScript has no external dependencies and is meant to be included as source into any project that needs it. CMake defines a macro `COMPILE_TESTS_MAIN` that makes it build its own `main()` funciton inside of `Test.cpp`
that serves as a minimal example and a minimal test, but when used as a middleware, there is no main function and you're expected to just add all of its `*.cpp` files to your project and include the `include/LegitScript.h` to use it.
