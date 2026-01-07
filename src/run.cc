#include <iostream>
#include <regex>
#include <run.hpp>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace std;

int run_func(const vector<string> &files, bool keep, bool plus,
             bool compile_only, const vector<string> &args)
{
  // cout << "DEBUG: run called" << endl;
  json conf = get_conf();
  bool to_o = compile_only;

  string extensions[] = {".cpp", ".cxx", ".cc"};

  bool use_cpp =
      plus || any_of(files.begin(), files.end(),
                     [&](const string &path_str)
                     {
                       fs::path p(path_str);
                       return any_of(begin(extensions), end(extensions),
                                     [&](const string &ext)
                                     { return p.extension() == ext; });
                     });
  // cout << "DEBUG: use_cpp = " << (use_cpp ? "oui" : "non") << endl;

  string mainFile = "";

  if (!to_o)
  {
    mainFile = findMainFile(files);

    if (!mainFile.empty())
    {
      cout << "DEBUG: Main function found in : " << mainFile << endl;
    }
    else
    {
      cout << "DEBUG: No main function found. Compiling to .o." << endl;
      to_o = true;
    }
  }
  // TODO : get inclusions
  // TODO : build command
  // TODO : clear if chosen in settings
  // TODO : display errors / warnings
  // TODO : launch program if compilation succeeded, handling interrupts
  // TODO : delete program if chosen

  return 0;
}
