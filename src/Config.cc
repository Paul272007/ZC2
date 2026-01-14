#include <fstream>

#include <Config.hh>
#include <File.hh>
#include <ZCError.hh>
#include <map>
#include <string>
#include <vector>

using namespace std;
using json = nlohmann::json;

Config::Config()
{
  fetch();
  lfetch();
}

Config::Config(bool user_or_libs)
{
  if (user_or_libs)
    lfetch();
  else
    fetch();
}

void Config::fetch()
{
  json json_conf;
  ifstream input(PATH);
  if (!input.is_open())
  {
    throw ZCError(3, "The config file couldn't be loaded.");
  }
  try
  {
    input >> json_conf;
  }
  catch (const json::parse_error &e)
  {
    string msg = "JSON parse error: ";
    msg += e.what();
    throw ZCError(1, msg);
  }
  c_compiler_ = json_conf.value("c_compiler", "gcc");
  cpp_compiler_ = json_conf.value("cpp_compiler", "g++");

  flags_ = json_conf.value<vector<string>>("flags",
                                           vector<string>{"-Wall", "-Wextra"});

  clear_before_run_ = json_conf.value<bool>("clear_before_run", true);
  auto_keep_ = json_conf.value<bool>("auto_keep", false);

  c_std_ = json_conf.value("c_std", "c99");
  cpp_std_ = json_conf.value("cpp_std", "c++17");
  include_dir_ = json_conf.value("include_dir", "/usr/local/include/");
  lib_dir_ = json_conf.value("lib_dir", "/usr/local/lib/");
  editor_ = json_conf.value("editor", "nano");
  open_on_init_ = json_conf.value("open_on_init", true);
}

void Config::lfetch()
{
  json json_conf;
  ifstream input(LPATH);
  if (!input.is_open())
  {
    throw ZCError(3, "The config file couldn't be loaded.");
  }
  try
  {
    input >> json_conf;
  }
  catch (const json::parse_error &e)
  {
    string msg = "JSON parse error: ";
    msg += e.what();
    throw ZCError(1, msg);
  }
  map<string, vector<vector<string>>> libraries_container =
      json_conf.value<map<string, vector<vector<string>>>>("libraries", {});

  for (const auto &[language, libs] : libraries_container)
  {
    libraries_[language];
    for (const auto &lib : libs)
    {
      Library l = {lib[0], lib[1], lib[2], lib[3]};
      libraries_[language].push_back(l);
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
    catch (exception &e)
    {
      string msg = "Configuration writing error: ";
      msg += e.what();
      throw ZCError(2, msg);
    }
  }
}

vector<vector<string>> Config::librariesToMatrix() const
{
  vector<vector<string>> v;
  v.push_back(
      {"Library name", "Compiling option", "Author", "Version", "Language"});

  for (const auto &[language, libs] : libraries_)
  {
    for (const auto &lib : libs)
    {
      vector<string> vect{lib.libname_, lib.compiling_option_, lib.author_,
                          lib.version_, language};
      v.push_back(vect);
    }
  }

  return v;
}
