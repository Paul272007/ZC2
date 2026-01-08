#ifndef CORE_H_
#define CORE_H_

// Includes

#include <exception>
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <regex>
#include <string>
#include <vector>
#include <zcerror.hpp>

// Macros

#define PATH "../etc/config.json"

using json = nlohmann::json;

// Structs

struct Config
{
  std::string c_compiler = "gcc";
  std::string cpp_compiler = "g++";
  std::vector<std::string> flags = {"-Wall", "-Wextra"};
  std::map<std::string, std::string> libraries = {{"math", "-lm"},
                                                  {"ncurses", "-lncurses"}};
  bool clear_before_run = true;
  bool auto_keep = false;
  std::string std = "c99";
  std::string include_dir = "/usr/local/include/";
  std::string lib_dir = "/usr/local/lib/";
};

struct Declarations
{
  std::vector<std::string> includes;
  std::vector<std::string> macros;
  std::vector<std::string> globals;
  std::vector<std::string> typedefs;
  std::vector<std::string> enums;
  std::vector<std::string> functions;
  std::vector<std::string> structs;
  std::vector<std::string> unions;
};

// Utilities

/**
 * @brief Escapes a string to be safely used as a shell argument.
 */
std::string escape_shell_arg(const std::string &arg);
/**
 * @brief Format and print a success message.
 */
void success(const std::string &msg);
/**
 * @brief Format and print a debugging message.
 */
void debug(const std::string &msg);
/**
 * @brief Format and print a warning.
 */
void warning(const std::string &msg);
/**
 * @brief Format and print an info.
 */
void info(const std::string &msg);
/**
 * @brief Get the configuration from the config file.
 */
Config get_conf();
/**
 * @brief Write the configuration to the config file.
 */
void write_conf(json conf);
/**
 * @brief Ask the user for a yes/no confirmation.
 * @return true if the user answered yes, false otherwise.
 */
[[nodiscard]] bool ask();
/**
 * @brief Find the main file from a list of files.
 */
std::string findMainFile(const std::vector<std::string> &files);
/**
 * @brief Split a string by a delimiter.
 */
std::vector<std::string> split(const std::string &s, char delimiter);
/**
 * @brief Convert a string to uppercase.
 */
std::string upper(std::string str);

// FileParser class

/**
 * @brief Class used to perform multiple actions on files, such as parsing
 */
class FileParser
{
public:
  explicit FileParser(const std::string &path);
  ~FileParser();
  /**
   * @brief Parse the file and extract declarations.
   */
  Declarations parse();
  /**
   * @brief Check if the file contains a main function.
   */
  bool containsMain();
  /**
   * @brief Get the inclusions from the file based on a map of libraries that
   * require flags when compiling.
   */
  std::vector<std::string>
  getInclusions(const std::map<std::string, std::string> &libs) const;
  /**
   * @brief Check if the file exists.
   */
  bool exists() const;
  /**
   * @brief Write content to the file.
   */
  bool writeContent(const std::string &content) const;
  /**
   * @brief Append content to the file.
   */
  bool appendContent(const std::string &content) const;
  /**
   * @brief Write the formatted declarations to the file.
   */
  bool writeDeclarations(const Declarations &decls,
                         const std::string &constant) const;

private:
  /**
   * @brief Reads the content of the file.
   */
  std::string readFile() const;
  /**
   * @brief Find all matches of a regex in a given text.
   */
  std::vector<std::string> findAll(const std::string &text,
                                   const std::regex &re);
  /**
   * @brief Remove comments from the given text.
   */
  std::string removeComments(const std::string &text);
  /**
   * @grief Extract code blocks based on a keyword (e.g., struct, enum).
   */
  std::vector<std::string> extractBlock(const std::string &content,
                                        const std::string &keyword);
  /**
   * @brief Path to the file being parsed.
   */
  std::string path_;
};

#endif // !CORE_H_
