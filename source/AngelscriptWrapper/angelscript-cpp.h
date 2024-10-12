#pragma once
#include <functional>
#include <memory>
#include <angelscript.h>
#include <string>
#include <stdexcept>
#include <scriptstdstring/scriptstdstring.h>

namespace as
{
  struct ScriptEngine
  {
    struct MessageCallbackBinding
    {
      using FuncType = std::function<void(const asSMessageInfo *msg)>;
      MessageCallbackBinding(FuncType func) : func(func) {}
      FuncType func;
    };

    static std::unique_ptr<ScriptEngine> Create(MessageCallbackBinding::FuncType message_func = nullptr)
    {
      return std::unique_ptr<ScriptEngine>(new ScriptEngine(message_func));
    }
    ~ScriptEngine()
    {
      ptr->ShutDownAndRelease();
    }
    asIScriptModule * LoadScript(std::string script)
    {
      asIScriptModule *mod = ptr->GetModule(0, asGM_ALWAYS_CREATE);
      
      int res = mod->AddScriptSection("script", script.data(), script.length());
      if(res < 0) throw std::runtime_error("Failed to add script section");
      res = mod->Build();      
      if( res < 0 ) throw std::runtime_error("Failed to build the script");
      return mod;
    }
    void RunScript(asIScriptFunction *func)
    {
      auto context = this->CreateContext();
      context->ptr->Prepare(func);
      //ctx->SetArgFloat(1, 2.71828182846f);
      int res = context->ptr->Execute();
      if( res != asEXECUTION_FINISHED )
      {
        // The execution didn't finish as we had planned. Determine why.
        if( res == asEXECUTION_ABORTED )
          throw std::runtime_error("Script aborted");
        else if( res == asEXECUTION_EXCEPTION )
        {
          std::string err;
          err = "The script ended with an exception.";

          // Write some information about the script exception
          asIScriptFunction *func = context->ptr->GetExceptionFunction();
          err += std::string("func: ") + std::string(func->GetDeclaration()) + "\n";
          err += std::string("modl: ") + std::string(func->GetModuleName()) + "\n";
          err += std::string("sect: ") + std::string(func->GetScriptSectionName()) + "\n";
          err += std::string("line: ") + std::to_string(context->ptr->GetExceptionLineNumber()) + "\n";
          err += std::string("desc: ") + std::string(context->ptr->GetExceptionString()) + "\n";
          throw std::runtime_error(err);
        }
        else
          throw std::runtime_error("Who the fuck knows what went wrong");
      }
    }
    
    struct GlobalFunctionBinding
    {
      using FuncType = std::function<void(asIScriptGeneric *gen)>;
      GlobalFunctionBinding(FuncType func) : func(func){}
      FuncType func;
    };
    template<typename T>
    void RegisterType(std::string type_name)
    {
      int res = this->ptr->RegisterObjectType(type_name.data(), sizeof(T), asOBJ_VALUE | asOBJ_POD);
      if(res < 0) throw std::runtime_error("Failed to register a type");
    }
    void RegisterEnum(std::string enum_name, const std::vector<std::pair<std::string, int>> &enum_values)
    {
      int res = this->ptr->RegisterEnum(enum_name.c_str());
      if(res < 0) throw std::runtime_error("Failed to register an enum");
      
      for(auto value : enum_values)
        this->ptr->RegisterEnumValue(enum_name.c_str(), value.first.c_str(), value.second);
    }

    void RegisterGlobalFunction(std::string func_decl, GlobalFunctionBinding::FuncType func)
    {
      global_func_bindings.emplace_back(std::unique_ptr<GlobalFunctionBinding>(new GlobalFunctionBinding(func)));
      int res = this->ptr->RegisterGlobalFunction(func_decl.c_str(), asFUNCTION(GlobalFunctionBindingDispatcher), asCALL_GENERIC, global_func_bindings.back().get());
      if(res < 0) throw std::runtime_error("Failed to register a global function");
    }
    void RegisterMethod(std::string obj_type_name, std::string method_decl, GlobalFunctionBinding::FuncType func)
    {
      global_func_bindings.emplace_back(std::unique_ptr<GlobalFunctionBinding>(new GlobalFunctionBinding(func)));
      int res = this->ptr->RegisterObjectMethod(obj_type_name.c_str(), method_decl.c_str(), asFUNCTION(GlobalFunctionBindingDispatcher), asCALL_GENERIC, global_func_bindings.back().get());
      if(res < 0) throw std::runtime_error("Failed to register a global function");
    }
    static void GlobalFunctionBindingDispatcher(asIScriptGeneric *gen)
    {
      auto binding = (GlobalFunctionBinding*)gen->GetAuxiliary();
      binding->func(gen);
    }
    static void MessageCallbackDispatcher(const asSMessageInfo *msg, void *param)
    {
      auto binding = (MessageCallbackBinding*)param;
      binding->func(msg);
    }

    asIScriptEngine *ptr;
  private:
    struct Context
    {
      Context(asIScriptEngine *eng)
      {
        ptr = eng->CreateContext();
        if(!ptr) throw std::runtime_error("Failed to create context");
      }
      ~Context()
      {
        ptr->Release();
      }
      asIScriptContext *ptr;
    }; 
    std::unique_ptr<Context> CreateContext()
    {
      return std::unique_ptr<Context>(new Context(this->ptr));
    }
    std::vector<std::unique_ptr<GlobalFunctionBinding>> global_func_bindings;

    ScriptEngine(MessageCallbackBinding::FuncType message_func)
    {
      ptr = asCreateScriptEngine();
      RegisterStdString(ptr);
      if(!ptr) throw std::runtime_error("Failed to start as engine");
      
      if(message_func)
      {
        message_callback_binding = std::unique_ptr<MessageCallbackBinding>(new MessageCallbackBinding(message_func));
        int res = ptr->SetMessageCallback(asFUNCTION(MessageCallbackDispatcher), message_callback_binding.get(), asCALL_CDECL);
        if(res < 0) throw std::runtime_error("Failed to set a callback");
      }
    }
    std::unique_ptr<MessageCallbackBinding> message_callback_binding;
  };
}