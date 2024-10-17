#include <string>
#include <memory>
#include <optional>

namespace ls
{
  struct SourceAssembler
  {
    SourceAssembler();
    ~SourceAssembler();
    void AddSourceBlock(const std::string &text, size_t start_line);
    void AddNonSourceBlock(const std::string &text);
    std::string GetSource();
    std::optional<size_t> GetSourceLine(size_t res_line);
  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
  };
}