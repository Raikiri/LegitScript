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

void PrintRequest(const ls::FloatRequest req)
{
}
void PrintRequest(const ls::IntRequest req)
{
}
void PrintRequest(const ls::ColorRequest req)
{
}
void PrintRequest(const ls::BoolRequest req)
{
}
void PrintRequest(const ls::TextRequest req)
{
}
void PrintRequest(const ls::LoadedImageRequest req)
{
}
void PrintRequest(const ls::CachedImageRequest req)
{
}

void PrintScriptEvents(const ls::ScriptEvents script_events)
{
  for(const auto &req : script_events.context_requests)
  {
    std::visit([](auto r){
      PrintRequest(r);
    }, req);
  }
  
  for(const auto &inv : script_events.script_shader_invocations)
    PrintShaderInvocation(inv);
}

void RunTest()
{
  std::cout << "Test starts\n";  
  ls::LegitScript script;
  
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
    auto script_events = script.RunScript({});
    std::cout << "Script ran successfully\n";
    PrintScriptEvents(script_events);
  }
  catch(const std::exception &e)
  {
    std::cout << "Exception: " << e.what();
  }
}


void RunTestJson()
{
  
  std::cout << "Test starts\n";  
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
    std::string script_calls = ls::RunScript("[{\"name\": \"@swapchain_size\", \"type\": \"uvec2\", \"value\":{\"x\":512, \"y\":512}}]");
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
