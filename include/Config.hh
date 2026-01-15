#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#define DEBUG_MODE

// clang-format off
#if defined(_WIN32) || defined(_WIN64)
  // On Windows, store config in the current directory or AppData
  #define PATH "config.json"
  #define LPATH "zc-lock.json"
#elif defined(__APPLE__)
  // Idk shit bout macOS
  // js hope it follows Unix standards
  #define PATH "/usr/local/etc/zc/config.json"
  #define LPATH "/usr/local/etc/zc/zc-lock.json"
#else
  #ifdef DEBUG_MODE
    #define PATH "../etc/config.json"
    #define LPATH "../etc/zc-lock.json"
  #else
    #define PATH "/etc/zc/config.json"
    #define LPATH "/etc/zc/zc-lock.json"
  #endif
#endif
// clang-format on

struct Library
{
  std::string libname_;

  /**
   * @brief Header files corresponding to this library
   */
  std::vector<std::string> headers_;

  /**
   * @brief Binaries (.a/.so) corresponding to this library
   */
  std::vector<std::string> binaries_;
  std::string compiling_option_;
  std::string version_;

  /**
   * @brief ID of the library's author
   */
  std::string author_;
};

class Config
{
public:
  /**
   * @brief The default constructor fetches the configuration for the libraries
   * and for the user settings
   */
  Config();

  /**
   * @brief Specify if we want only the user configuration or the libraries
   * configuration
   * @param user_or_libs true for libraries configuration fales for user
   * configuration
   */
  Config(bool user_or_libs);

  /**
   * @brief Write the given configuration into the user settings configuration
   * file
   */
  void write();

  /**
   * @brief Turn libraries_ into a matrix with the first line being a header
   * line (to display the information in a table for example)
   */
  [[nodiscard]] std::vector<std::vector<std::string>> librariesToMatrix() const;

  /* User settings */
  std::string c_compiler_ = "gcc";
  std::string cpp_compiler_ = "g++";
  std::vector<std::string> flags_ = {"-Wall", "-Wextra"};
  bool clear_before_run_ = true;
  bool auto_keep_ = false;
  std::string c_std_ = "c99";
  std::string cpp_std_ = "c++17";
  std::string include_dir_ = "/usr/local/include/";
  std::string lib_dir_ = "/usr/local/lib/";
  std::string editor_ = "nano";
  bool open_on_init_ = true;

  /* Library configuration */

  /**
   * @brief language => list of libraries
   */
  std::map<std::string, std::vector<Library>> libraries_;

private:
  void fetch();
  void lfetch();
};

#endif // !CONFIG_H_
