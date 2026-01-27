#include <iostream>

#include <commands/Lib/List.hh>
#include <objects/Registry.hh>
#include <zcio.hh>

using namespace std;

List::List(bool display_std)
    : registry_(Registry::getInstance()), display_std_(display_std)
{
}

int List::execute()
{
  if (display_std_)
  {
    Table std_lib = registry_.stdPackagesTable();

    if (std_lib.getSize() < 2)
    {
      cout << "No standard libraries installed" << endl;
      return 0;
    }

    std_lib.draw();
    return 0;
  }

  Table lib = registry_.packagesTable();

  if (lib.getSize() < 2)
  {
    cout << "No libraries installed" << endl;
    return 0;
  }

  lib.draw();
  return 0;
}
