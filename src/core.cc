#include <core.hpp>

using namespace std;

json get_conf() {
  json config;
  ifstream input(PATH);
  if (input.is_open()) {
    try {
      input >> config;
    } catch (json::parse_error &e) {
      string msg = "JSON parse error: ";
      msg += e.what();
      throw ZCError(1, msg);
    }
  }
  return config;
}

void write_conf(json conf) {
  ofstream output(PATH);
  if (output.is_open()) {
    try {
      output << setw(4) << conf << endl;
    } catch (exception &e) {
      string msg = "configuration writing: ";
      msg += e.what();
      throw ZCError(2, msg);
    }
  }
}

bool ask() {
  char input = 'Y';
  cout << "[Y/n] ";
  cin >> input;
  while (input != 'Y' && input != 'n') {
    cout << "Error: unexpected token" << endl << "[Y/n] ";
    cin >> input;
  }
  return input == 'Y';
}
