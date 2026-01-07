#include <run.hpp>

namespace fs = std::filesystem;
using namespace std;

int run_func(const vector<string> &files, bool keep, bool plus,
             bool compile_only, const vector<string> &args) {
  // cout << "DEBUG: run called" << endl;
  json conf = get_conf();

  string extensions[] = {".cpp", ".cxx", ".cc"};

  bool use_cpp =
      plus || any_of(files.begin(), files.end(), [&](const string &path_str) {
        fs::path p(path_str);
        return std::any_of(
            begin(extensions), end(extensions),
            [&](const std::string &ext) { return p.extension() == ext; });
      });
  // cout << "DEBUG: use_cpp = " << (use_cpp ? "oui" : "non") << endl;
  // TODO : get main function
  // TODO : get inclusions
  // TODO : build command
  // TODO : clear if chosen in settings
  // TODO : display errors / warnings
  // TODO : launch program if compilation succeeded, handling interrupts
  // TODO : delete program if chosen

  return 0;
}
