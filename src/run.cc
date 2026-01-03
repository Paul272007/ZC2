#include <run.hpp>

using namespace std;

int run_func(const vector<string> &files, bool keep, bool plus,
             bool compile_only, const vector<string> &args) {
  cout << "DEBUG: run called" << endl;
  json conf = get_conf();
  string extensions[] = {".cpp", ".cxx", ".cc"};
  bool use_cpp = (plus) ? plus : plus;
  return 0;
}
