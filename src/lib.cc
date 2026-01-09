#include <core.hpp>
#include <display.h>
#include <iostream>
#include <lib.hpp>
#include <utility>

using namespace std;

static pair<int, int> max_size(map<string, string> &arg)
{
  int max_key = 0;
  int max_value = 0;
  for (const auto &library : arg)
  {
    if (library.first.size() > max_key)
      max_key = library.first.size();
    if (library.second.size() > max_value)
      max_value = library.second.size();
  }
  return pair<int, int>(max_key, max_value);
}

int lib_list()
{
  // cout << "DEBUG: lib_list called" << endl;
  // Fetch configuration
  Config conf = get_conf();
  if (conf.libraries.empty())
  {
    cout << "No libraries installed." << endl;
    return 0;
  }
  // Get the length of the table columns = the length of the longest libname
  const auto max_sizes = max_size(conf.libraries);
  const char *headers[] = {"Library name", "Compiling option"};
  const int header_lengths[] = {len(headers[0]), len(headers[1])};
  // Add padding and transfer to an array to use in the C program
  const int widths[2] = {
      (max_sizes.first > header_lengths[0]) ? max_sizes.first + 2
                                            : header_lengths[0] + 2,
      (max_sizes.second > header_lengths[1]) ? max_sizes.second + 2
                                             : header_lengths[1] + 2};
  // Header names
  table_top_line(2, widths);
  table_middle_line(2, headers, widths);
  separator(2, widths);
  for (const auto &library : conf.libraries)
  {
    const char *values[] = {library.first.c_str(), library.second.c_str()};
    table_middle_line(2, values, widths);
  }
  table_bottom_line(2, widths);
  return 0;
}

int lib_install(string h, string a, bool force)
{
  cout << "DEBUG: lib_install called" << endl;
  return 0;
}

int lib_create(vector<string> files, bool force)
{
  cout << "DEBUG: lib_create called" << endl;
  return 0;
}

int lib_remove(string lib)
{
  cout << "DEBUG: lib_remove called" << endl;
  return 0;
}
