#include <core.hpp>
#include <cstdlib>
#include <display.h>
#include <iostream>
#include <lib.hpp>
#include <string>

using namespace std;

int lib_list()
{
  // Fetch configuration
  Config conf = get_conf();
  if (conf.libraries.empty())
  {
    cout << "No libraries installed." << endl;
    return 0;
  }
  int n_rows = conf.libraries.size() + 1;
  int n_cols = 2;
  // Allocate the temporary char*** for setTableContent
  char ***tmpContent = (char ***)malloc(n_rows * sizeof(char **));
  for (int i = 0; i < n_rows; i++)
  {
    tmpContent[i] = (char **)malloc(n_cols * sizeof(char *));
  }
  string h1("Library name");
  string h2("Compiling option");
  tmpContent[0][0] = (char *)h1.c_str();
  tmpContent[0][1] = (char *)h2.c_str();
  int current_row = 1;
  for (auto const &[key, val] : conf.libraries)
  {
    tmpContent[current_row][0] = (char *)key.c_str();
    tmpContent[current_row][1] = (char *)val.c_str();
    current_row++;
  }
  Table *TablePtr = newTable(n_rows, n_cols, 0, 1);
  setTableThickness(TablePtr, 1, 1, 2, 2, 2, 2);
  setTableContent(TablePtr, tmpContent);
  drawTable(TablePtr);
  for (int i = 0; i < n_rows; ++i)
    free(tmpContent[i]);
  free(tmpContent);
  deleteTable(TablePtr);
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
