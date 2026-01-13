#include <fstream>

#include <Config.hh>
#include <File.hh>
#include <ZCError.hh>

using namespace std;
using json = nlohmann::json;

Config::Config() { fetch(); }

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
  catch (const exception &e)
  {
    cerr << e.what() << endl;
  }
  c_compiler_ = json_conf.value("c_compiler", "gcc");
  cpp_compiler_ = json_conf.value("cpp_compiler", "g++");

  flags_ = json_conf.value<vector<string>>("flags",
                                           vector<string>{"-Wall", "-Wextra"});
  libraries_ =
      json_conf.value<map<string, string>>("libraries", {{"math", "-lm"}});

  clear_before_run_ = json_conf.value<bool>("clear_before_run", true);
  auto_keep_ = json_conf.value<bool>("auto_keep", false);

  c_std_ = json_conf.value("c_std", "c99");
  cpp_std_ = json_conf.value("cpp_std", "c++17");
  include_dir_ = json_conf.value("include_dir", "/usr/local/include/");
  lib_dir_ = json_conf.value("lib_dir", "/usr/local/lib/");
  editor_ = json_conf.value("editor", "nano");
  open_on_init_ = json_conf.value("open_on_init", true);
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
      string msg = "configuration writing: ";
      msg += e.what();
      throw ZCError(2, msg);
    }
  }
}
