#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include <Config.hh>
#include <File.hh>
#include <ZCError.hh>
#include <helpers.hh>

using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

Config::Config() : Config(false) {} // Use main constructor

Config::Config(bool user_or_libs)
{
  fetch();
  // Load libs only if necessary to prevent boot errors
  if (user_or_libs)
    lfetch();
}

void Config::fetch()
{
  json json_conf;

  // 1. Define root directory (~/.zc per default)
  const char *home = getenv("HOME");
  string default_prefix = (home ? string(home) : ".") + "/.zc";

  if (fs::exists(PATH))
  {
    ifstream input(PATH);
    if (input.is_open())
      try
      {
        input >> json_conf;
      }
      catch (const json::parse_error &e)
      {
        string msg = "JSON parse error: ";
        msg += e.what();
        throw ZCError(ZC_CONFIG_PARSING_ERROR, msg);
      }

    else
      throw ZCError(ZC_CONFIG_NOT_FOUND, "The config file couldn't be loaded.");
  }
  // Compilers configuration
  c_compiler_ = json_conf.value("c_compiler", "gcc");
  cpp_compiler_ = json_conf.value("cpp_compiler", "g++");

  flags_ = json_conf.value<vector<string>>("flags",
                                           vector<string>{"-Wall", "-Wextra"});
  c_std_ = json_conf.value("c_std", "c99");
  cpp_std_ = json_conf.value("cpp_std", "c++17");

  // Paths
  install_prefix_ = json_conf.value("install_prefix", default_prefix);
  include_dir_ = json_conf.value("include_dir", install_prefix_ + "/include/");
  lib_dir_ = json_conf.value("lib_dir", install_prefix_ + "/lib/");

  try
  {
    if (!fs::exists(include_dir_))
      fs::create_directories(include_dir_);
    if (!fs::exists(lib_dir_))
      fs::create_directories(lib_dir_);
  }
  catch (...) // Ignore errors for the moment
  {
  }

  // User configuration
  clear_before_run_ = json_conf.value<bool>("clear_before_run", true);
  auto_keep_ = json_conf.value<bool>("auto_keep", false);
  editor_ = json_conf.value("editor", "nano");
  open_on_init_ = json_conf.value("open_on_init", true);
}

void Config::lfetch()
{
  json json_conf;
  ifstream input(LPATH);
  if (!input.is_open())
  {
    throw ZCError(ZC_CONFIG_NOT_FOUND, "The config file couldn't be loaded.");
  }
  try
  {
    input >> json_conf;
  }
  catch (const json::parse_error &e)
  {
    string msg = "JSON parse error: ";
    msg += e.what();
    throw ZCError(ZC_CONFIG_PARSING_ERROR, msg);
  }

  if (json_conf.contains("libraries") && json_conf["libraries"].is_object())
  {
    for (const auto &[language, libs] : json_conf["libraries"].items())
    {
      for (const auto &lib : libs)
      {
        if (lib.is_array() && lib.size() >= 6)
        {
          // clang-format off
          Library l;
          l.libname_          = lib.at(0).get<string>();
          l.headers_          = lib.at(1).get<vector<string>>();
          l.binaries_         = lib.at(2).get<vector<string>>();
          l.compiling_option_ = lib.at(3).get<string>();
          l.version_          = lib.at(4).get<string>();
          l.author_           = lib.at(5).get<string>();
          // clang-format on

          libraries_[language].push_back(l);
        }
      }
    }
  }
}

void Config::write()
{
  // TODO : transform Config into json
  ofstream output(PATH);
  if (output.is_open())
  {
    try
    {
      output << setw(4) << endl;
    }
    catch (const exception &e)
    {
      string msg = "Configuration writing error: ";
      msg += e.what();
      throw ZCError(ZC_CONFIG_NOT_FOUND, msg);
    }
  }
}

vector<vector<string>> Config::librariesToMatrix() const
{
  vector<vector<string>> v;
  v.push_back({"Library name", "Header files", "Binaries", "Compiling option",
               "Author", "Version", "Language"});

  for (const auto &[language, libs] : libraries_)
  {
    for (const auto &lib : libs)
    {
      vector<string> vect{lib.libname_,
                          join(lib.headers_, ", "),
                          join(lib.binaries_, ", "),
                          lib.compiling_option_,
                          lib.author_,
                          lib.version_,
                          language};
      v.push_back(vect);
    }
  }

  return v;
}
