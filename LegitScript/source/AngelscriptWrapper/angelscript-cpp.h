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
    
    struct RuntimeException
    {
      size_t line_number;
      std::string func_decl;
      std::string exception_str;
    };
    std::optional<RuntimeException> RunScript(asIScriptFunction *func)
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
          asIScriptFunction *func = context->ptr->GetExceptionFunction();
          RuntimeException exc;
          exc.line_number = context->ptr->GetExceptionLineNumber();
          exc.func_decl = func->GetDeclaration();
          exc.exception_str = context->ptr->GetExceptionString();
          return exc;
        }
        else
          throw std::runtime_error("Who the fuck knows what went wrong");
      }
      return std::nullopt;
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
    void RegisterMember(std::string obj_type_name, std::string member_decl, size_t offset)
    {
      int res = this->ptr->RegisterObjectProperty(obj_type_name.c_str(), member_decl.c_str(), offset);
      if(res < 0) throw std::runtime_error("Failed to register a member");
    }
    void RegisterConstructor(std::string obj_type_name, std::string constr_decl, GlobalFunctionBinding::FuncType func)
    {
      global_func_bindings.emplace_back(std::unique_ptr<GlobalFunctionBinding>(new GlobalFunctionBinding(func)));
      int res = this->ptr->RegisterObjectBehaviour(obj_type_name.c_str(), asBEHAVE_CONSTRUCT, constr_decl.c_str(), asFUNCTION(GlobalFunctionBindingDispatcher), asCALL_GENERIC, global_func_bindings.back().get());
      if(res < 0) throw std::runtime_error("Failed to register a constructor");
    }
    void RegisterDestructor(std::string obj_type_name, GlobalFunctionBinding::FuncType func)
    {
      global_func_bindings.emplace_back(std::unique_ptr<GlobalFunctionBinding>(new GlobalFunctionBinding(func)));
      int res = this->ptr->RegisterObjectBehaviour(obj_type_name.c_str(), asBEHAVE_DESTRUCT, "void f()", asFUNCTION(GlobalFunctionBindingDispatcher), asCALL_GENERIC, global_func_bindings.back().get());
      if(res < 0) throw std::runtime_error("Failed to register a destructor");
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
    static void ExceptionCallbackDispatcher(asIScriptContext *ctx, void *param)
    {
      try
      {
        // Retrow the original exception so we can catch it again higher up
        throw;
      }
      catch(const std::exception &e)
      {
        ctx->SetException(e.what());
      }
      catch(...)
      {
      }
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
        if(res < 0) throw std::runtime_error("Failed to set a message callback");
      }
      {
        int res = ptr->SetTranslateAppExceptionCallback(asFUNCTION(ExceptionCallbackDispatcher), nullptr, asCALL_CDECL);
        if(res < 0) throw std::runtime_error("Failed to set an exception callback");
      }
    }
    std::unique_ptr<MessageCallbackBinding> message_callback_binding;
  };
}