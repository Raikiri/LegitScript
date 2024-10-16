#include <LegitScript.h>
#include <LegitScriptJsonApi.h>
#include <iostream>
#include <fstream>
#include <sstream>

void PrintShaderDesc(const ls::ShaderDesc &shader_desc)
{
  std::cout << "Shader desc: " << shader_desc.name << "\n";
  std::cout << "  Uniforms:\n";
  for(const auto &uniform : shader_desc.uniforms)
    std::cout << "    " << uniform.type << " " << uniform.name << "\n";
  std::cout << "  Samplers:\n";
  for(const auto &sampler : shader_desc.samplers)
    std::cout << "    " << sampler.type << " " << sampler.name << "\n";
  std::cout << "  Render targets:\n";
  for(const auto &out : shader_desc.outs)
    std::cout << "    " << out.type << " " << out.name << "\n";
}

void PrintCachedImgRequest(const ls::CachedImageRequest &req)
{
  std::cout << "Image request: [" << req.size.x << ", " << req.size.y << "]\n";
}
void PrintShaderInvocation(const ls::ShaderInvocation &inv)
{
  std::cout << "Shader invocation: " << inv.shader_name << "\n";
}
void PrintScriptEvents(const ls::ScriptEvents script_events)
{
  for(const auto &req : script_events.cached_image_requests)
    PrintCachedImgRequest(req);
  
  for(const auto &inv : script_events.script_shader_invocations)
    PrintShaderInvocation(inv);
}

void RunTest()
{
  std::cout << "Test starts\n";  
  ls::LegitScript script(
    [](std::string name, float val, float min_val, float max_val) -> float
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
    }
  );
  
  std::ifstream file_stream("../data/Scripts/main.ls");
  std::stringstream string_stream;
  string_stream << file_stream.rdbuf();
  std::cout << "Loading script size: " << string_stream.str().length() << "\n";
  try
  {
    auto script_contents = script.LoadScript(string_stream.str());
    std::cout << "Script loaded successfully\n";
    for(const auto &shader_desc : script_contents.shader_descs)
      PrintShaderDesc(shader_desc);
      
    std::cout << "Running script\n";
    auto script_calls = script.RunScript({1024, 1024}, 0.0f);
    std::cout << "Script ran successfully\n";
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


void RunTestJson()
{
  
  std::cout << "Test starts\n";  
  ls::InitScript([](std::string name, float val, float min_val, float max_val) -> float
    {
      std::cout << "Slider float: " << val << "[" << min_val << ", " << max_val << "]\n";
      return val;
    },
    [](std::string name, int val, int min_val, int max_val) -> int
    {
      std::cout << "Slider int: " << val << "[" << min_val << ", " << max_val << "]\n";
      return val;
    },
    [](std::string text) -> void
    {
      std::cout << "Text: " << text << "\n";
    });

  std::ifstream file_stream("../data/Scripts/main.ls");
  std::stringstream string_stream;
  string_stream << file_stream.rdbuf();
  std::cout << "Loading script size: " << string_stream.str().length() << "\n";
  try
  {
    std::string shader_descs = ls::LoadScript(string_stream.str());
    std::cout << "Script loaded successfully\n";
    std::cout << "Shader descs: " << shader_descs << "\n";
      
    std::cout << "Running script\n";
    std::string script_calls = ls::RunScript(1024, 1024, 0.0f);
    std::cout << "Script ran successfully: \n";
    std::cout << "Script calls: " << script_calls << "\n";
  }
  catch(const std::exception &e)
  {
    std::cout << "Exception: " << e.what();
  }
}

int main()
{
  //RunTest();
  RunTestJson();
  return 0;
}
