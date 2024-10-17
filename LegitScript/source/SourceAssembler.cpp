#include "../include/SourceAssembler.h"
#include <vector>

namespace ls
{
  size_t GetLinesCount(const std::string &str)
  {
    size_t lines_count = 0;
    for(auto c : str)
    {
      if(c == '\n') lines_count++;
    }
    return lines_count;
  }
  struct SourceAssembler::Impl
  {
    void AddSourceBlock(const std::string &text, size_t start_line)
    {
      res_source += text;
      blocks.push_back({start_line, GetLinesCount(text)});
    }
    void AddNonSourceBlock(const std::string &text)
    {
      res_source += text;
      blocks.push_back({std::nullopt, GetLinesCount(text)});
    }
    std::string GetSource()
    {
      return res_source;
    }
    std::optional<size_t> GetSourceLine(size_t res_line)
    {
      size_t curr_block_start = 1;
      for(const auto &block : blocks)
      {
        if(res_line < curr_block_start + block.line_count)
        {
          if(block.start_line)
            return block.start_line.value() + (res_line - curr_block_start);
          else
            return std::nullopt;
        }
        curr_block_start += block.line_count;
      }
      return std::nullopt;
    }
    
    struct SourceBlock
    {
      std::optional<size_t> start_line;
      size_t line_count;
    };
    std::vector<SourceBlock> blocks;
    std::string res_source;
  };
  
  SourceAssembler::SourceAssembler()
  {
    impl.reset(new SourceAssembler::Impl());
  }
  SourceAssembler::~SourceAssembler()
  {
  }
  void SourceAssembler::AddSourceBlock(const std::string &text, size_t start_line)
  {
    impl->AddSourceBlock(text, start_line);
  }
  void SourceAssembler::AddNonSourceBlock(const std::string &text)
  {
    impl->AddNonSourceBlock(text);
  }
  std::string SourceAssembler::GetSource()
  {
    return impl->GetSource();
  }

  std::optional<size_t> SourceAssembler::GetSourceLine(size_t res_line)
  {
    return impl->GetSourceLine(res_line);
  }
  
}