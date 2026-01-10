#ifndef RUN_H_
#define RUN_H_

#include <core.hpp>
#include <string>
#include <vector>

class Builder
{
public:
  Builder(const Config &conf);
  ~Builder();
  /**
   * @brief Run the command
   */
  int run(const std::vector<std::string> &files,
          const std::vector<std::string> &args, bool keep, bool plus,
          bool compile_only) const;
  std::vector<std::string>
  getInclusions(const std::vector<std::string> &files) const;

private:
  Config conf_;
  bool hasCppExt(const std::vector<std::string> &files) const;
  std::string buildCommand(const std::vector<std::string> &files,
                           const std::vector<std::string> &libs, bool is_cpp,
                           const std::string &output_name) const;
  std::string buildCompileCommand(const std::vector<std::string> &files,
                                  bool is_cpp,
                                  const std::string &output_name) const;
};

int run_func(const std::vector<std::string> &files, bool keep, bool plus,
             bool compile_only, const std::vector<std::string> &args);

#endif // !RUN_H_
