#ifndef CONFIG_H_
#define CONFIG_H_

#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

// clang-format off
#if defined(_WIN32) || defined(_WIN64)
  // On Windows, store config in the current directory or AppData
  #define PATH "config.json"
#elif defined(__APPLE__)
  // Idk shit bout macOS
  // js hope it follows Unix standards
  #define PATH "/usr/local/etc/zc/config.json"
#else
  #define PATH "/etc/zc/config.json"
#endif
// clang-format on

class Config
{
public:
  Config();
  void write();
  std::string c_compiler_ = "gcc";
  std::string cpp_compiler_ = "g++";
  std::vector<std::string> flags_ = {"-Wall", "-Wextra"};
  std::map<std::string, std::string> libraries_ = {{"math", "-lm"},
                                                   {"ncurses", "-lncurses"}};
  bool clear_before_run_ = true;
  bool auto_keep_ = false;
  std::string c_std_ = "c99";
  std::string cpp_std_ = "c++17";
  std::string include_dir_ = "/usr/local/include/";
  std::string lib_dir_ = "/usr/local/lib/";

private:
  void fetch();
};

#endif // !CONFIG_H_
