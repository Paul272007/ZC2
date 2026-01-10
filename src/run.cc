#include "display.h"
#include <algorithm>
#include <core.hpp>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <run.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace std;

Builder::Builder(const Config &conf) : conf_(conf) {}
Builder::~Builder() {}

int Builder::run(const vector<string> &files, const vector<string> &args,
                 bool keep, bool plus, bool compile_only) const
{
  bool is_cpp = plus || this->hasCppExt(files);
  string output_name = "", cmd = "";
  int height, width;

  if (compile_only)
  {
    output_name = fs::path(files[0]).replace_extension(".o").string();
    cmd = buildCompileCommand(files, is_cpp, output_name);
  }
  else
  {
    output_name = findMainFile(files);
    // info("Main file found: " + main_program_path);
    debug("Main file found: " + output_name);
    // Get library inclusions for linker
    vector<string> needed_libs = this->getInclusions(files);
    cmd = buildCommand(files, needed_libs, is_cpp, output_name);
  }
  if (output_name.empty())
    throw ZCError(3, "No main function found in the provided files.");

  debug("Build command: " + cmd);
  // info("Build command: " + cmd);
  get_term_size(&height, &width);
  line(width);
  cout << flush;
  int compile_res = system(cmd.c_str());

  if (compile_res != 0)
  {
    line(width);
    throw ZCError(4, "Compilation failed.");
  }

  if (compile_only)
  {
    // No execution needed here
    success("Compilation successful.");
    success("Object file created: " + output_name);
    return 0;
  }

  /*
  if (conf_.clear_before_run)
    system("clear");
  */
  success("Compilation successful.");
  info("Executing program...");

  // Get absolute path of output binary
  string exec_cmd = fs::absolute(output_name).string();
  for (const auto &arg : args)
  {
    exec_cmd += " " + escape_shell_arg(arg);
  }

  // debug("Execution command: " + exec_cmd);
  int run_res = system(exec_cmd.c_str());

  if (!conf_.auto_keep && !keep && fs::exists(output_name))
  {
    fs::remove(output_name);
    debug("Temporary binary file removed: " + output_name);
  }
  return (run_res == 0) ? 0 : 5;
}

bool Builder::hasCppExt(const vector<string> &files) const
{
  for (const auto &file : files)
  {
    string ext = fs::path(file).extension().string();
    if (ext == ".cpp" || ext == ".cc" || ext == ".cxx" || ext == ".hpp")
    {
      return true;
    }
  }
  return false;
}

vector<string> Builder::getInclusions(const vector<string> &files) const
{
  // Contains all flags (for all files)
  vector<string> flags;

  // Search for flags to add in each file
  for (const auto &filepath : files)
  {
    FileParser f = FileParser(filepath);
    vector<string> file_flags = f.getInclusions(conf_.libraries);
    for (const auto &flag : file_flags)
    {
      if (find(flags.begin(), flags.end(), flag) == flags.end())
      {
        flags.push_back(flag);
      }
    }
  }
  return flags;
}

string Builder::buildCompileCommand(const vector<string> &files, bool is_cpp,
                                    const string &output_name) const
{
  stringstream cmd;
  // Compiler and Standard
  if (is_cpp)
    cmd << conf_.cpp_compiler << " -std=" << conf_.cpp_std << " ";
  else
    cmd << conf_.c_compiler << " -std=" << conf_.c_std << " ";

  // Global flags
  for (const auto &flag : conf_.flags)
    cmd << flag << " ";

  // Source files
  for (const auto &file : files)
    cmd << escape_shell_arg(file) << " ";

  // -c flag for compile_only
  cmd << "-c ";

  // Output
  cmd << "-o " << output_name << " ";

  // Color flags (gcc/g++)
  cmd << "-fdiagnostics-color=always";
  return cmd.str();
}

string Builder::buildCommand(const vector<string> &files,
                             const vector<string> &libs, bool is_cpp,
                             const string &output_name) const
{
  stringstream cmd;
  // Compiler
  cmd << (is_cpp ? conf_.cpp_compiler : conf_.c_compiler) << " ";

  // Standard
  cmd << "-std=" << (is_cpp ? conf_.cpp_std : conf_.c_std) << " ";

  // Global flags
  for (const auto &flag : conf_.flags)
  {
    cmd << flag << " ";
  }

  // Source files
  for (const auto &file : files)
  {
    cmd << escape_shell_arg(file) << " ";
  }

  // Output
  cmd << "-o " << output_name << " ";

  // libraries
  for (const auto &lib : libs)
  {
    cmd << lib << " ";
  }

  // Color flags (gcc/g++)
  cmd << "-fdiagnostics-color=always";
  return cmd.str();
}

int run_func(const vector<string> &files, bool keep, bool plus,
             bool compile_only, const vector<string> &args)
{
  Builder b = Builder(get_conf());
  return b.run(files, args, keep, plus, compile_only);
}
