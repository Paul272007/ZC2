#include <lib.hpp>

using namespace std;

int lib_list() {
  cout << "DEBUG: lib_list called" << endl;
  return 0;
}

int lib_install(string h, string a, bool force) {
  cout << "DEBUG: lib_install called" << endl;
  return 0;
}

int lib_create(vector<string> files, bool force) {
  cout << "DEBUG: lib_create called" << endl;
  return 0;
}

int lib_remove(string lib) {
  cout << "DEBUG: lib_remove called" << endl;
  return 0;
}
