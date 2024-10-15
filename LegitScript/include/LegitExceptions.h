#pragma once
#include <stdexcept>

namespace ls
{
  class ScriptException : public std::runtime_error
  {
  public:
    ScriptException(
      size_t line,
      size_t column,
      std::string func,
      std::string desc) :
      line(line),
      column(column),
      func(func),
      desc(desc),
      std::runtime_error(std::string("[") + std::to_string(line) + ":" + std::to_string(column) + "]" + func + ":" + desc){}

    size_t line;
    size_t column;
    std::string func;
    std::string desc;
  };
}