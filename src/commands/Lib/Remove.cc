#include <string>
#include <vector>

#include <commands/Lib/Remove.hh>
#include <objects/Registry.hh>
#include <zcio.hh>

Remove::Remove(const std::vector<std::string> &targets)
    : targets_(targets), registry_(Registry::getInstance())
{
}

int Remove::execute()
{
  for (const auto &pkg : targets_)
  {
    if (!registry_.removePackage(pkg))
      warning("All headers / binaries for package " + pkg +
              " weren't deleted successfully.");
  }
  return 0;
}
