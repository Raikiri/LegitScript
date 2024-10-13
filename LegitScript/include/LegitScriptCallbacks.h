#include <functional>
#include <string>

namespace ls
{
  using SliderFloatFunc = std::function<float(std::string, float, float, float)>;
  using SliderIntFunc = std::function<int(std::string, int, int, int)>;
  using TextFunc = std::function<void(std::string)>;
}