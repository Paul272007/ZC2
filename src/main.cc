#include <CLI11.hpp>

int main(int argc, char *argv[])
{
  CLI::App app{"ZC, a C utility to perform all actions on C and C++ files"};
  argv = app.ensure_utf8(argv);
  app.set_version_flag("--version,-v", "ZC v0.0.1 (dev)");
  app.require_subcommand(1);
  return 0;
}
