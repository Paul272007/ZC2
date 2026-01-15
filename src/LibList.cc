#include <iostream>
#include <string>
#include <vector>

#include <LibList.hh>
#include <Table.hh>
#include <helpers.hh>

using namespace std;

LibList::LibList() {}

LibList::~LibList() {}

int LibList::execute()
{
  vector<vector<string>> libs = config_.librariesToMatrix();
  // < 2 Because of header column
  if (libs.size() < 2)
  {
    cout << "No libraries installed." << endl;
    return 0;
  }

  int n_rows = libs.size();
  int n_cols = 6;

  Table t(n_rows, n_cols, false, true, libs);
  t.draw();

  return 0;
}
