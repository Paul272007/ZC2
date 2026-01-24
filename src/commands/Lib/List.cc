#include <iostream>

#include <commands/Lib/List.hh>
#include <objects/Registry.hh>
#include <zcio.hh>

using namespace std;

List::List() : registry_(Registry::getInstance()) {}

int List::execute()
{
  Table lib = registry_.packagesTable();

  if (lib.getSize() < 2)
  {
    cout << "No libraries installed" << endl;
    return 0;
  }

  lib.draw();
  return 0;
}
