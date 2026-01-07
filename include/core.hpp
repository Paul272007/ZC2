#ifndef CORE_H_
#define CORE_H_

#include <exception>
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex>
#include <string>
#include <vector>
#include <zcerror.hpp>

#define PATH "../etc/config.json"

using json = nlohmann::json;

json get_conf();
void write_conf(json conf);
[[nodiscard]] bool ask();

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

class FileParser
{
public:
  explicit FileParser(const std::string &path);
  Declarations parse();
  bool containsMain();

private:
  std::string readFile();
  std::vector<std::string> findAll(const std::string &text,
                                   const std::regex &re);
  std::string removeComments(const std::string &text);
  std::vector<std::string> extractBlock(const std::string &content,
                                        const std::string &keyword);
  std::string path_;
};

std::string findMainFile(const std::vector<std::string> &files);

#endif // !CORE_H_
