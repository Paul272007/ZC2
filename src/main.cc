#include <exception>
#include <memory>
#include <string>
#include <vector>

#include <CLI11.hpp>

#include <commands/Command.hh>
#include <commands/Init.hh>
#include <commands/Lib/Create.hh>
#include <commands/Lib/List.hh>
#include <commands/Project.hh>
#include <commands/Run.hh>
#include <objects/ZCError.hh>
#include <zcio.hh>

using namespace std;

int main(int argc, char *argv[])
{
  // Initialize CLI11 App
  CLI::App app{"ZC, a C utility to perform all actions on C and C++ files"};
  argv = app.ensure_utf8(argv);
  app.set_version_flag("--version,-v", "ZC v0.0.1 (dev)");
  app.require_subcommand(1);

  unique_ptr<Command> command(nullptr);

  /* ========================================================= *
   * Variables to store CLI arguments for Command constructors *
   * ========================================================= */

  // ========================= All Commands
  bool force = false;
  vector<string> input_files;

  // ========================= RUN
  bool run_keep = false, run_plus = false;
  bool run_c = false, run_S = false, run_E = false;

  vector<string> run_args;

  // ========================= LIB CREATE
  string pkg_name;

  // ========================= INIT
  vector<string> new_files;
  bool edit;

  //  ========================= PROJECT
  string language;
  string project_name;

  /* ========================================================= *
   *                         SUBCOMMANDS                       *
   * ========================================================= */

  // clang-format off
  auto run     = app.add_subcommand("run", "Compile and execute C/C++ file(s)");
  auto lib     = app.add_subcommand("lib", "Operations on libraries");
  auto init    = app.add_subcommand("init", "Initialize file(s) with a template");
  auto project = app.add_subcommand("project", "Initiliaze a new C/C++ project");

  /*
   * ========================== RUN ===============================
   */

  run->add_option("files", input_files, "The files to be compiled (and executed)")->required();
  run->add_option("--args,-a", run_args, "Arguments to be passed to the program when executing");

  run->add_flag("--keep,-k", run_keep, "Do not delete the executable after program ends");
  run->add_flag("--plus,-p", run_plus, "Force compilation as C++");
  run->add_flag("-E", run_E, "Preprocess only");
  run->add_flag("-S", run_S, "Compile, but do not assemble or link");
  run->add_flag("-c", run_c, "Compile and assemble, but do not link");

  run->callback([&]() { command = make_unique<Run>(input_files, run_args, run_keep, run_plus, run_E, run_S, run_c); });


  /*
   * ========================== INIT ===============================
   */

  init->add_option("files", new_files, "The files to be created and initialized")->required();
  init->add_option("--input,-i", input_files, "Files to be used as basis to write the new files");

  init->add_flag("--force,-f", force, "Force writing into the files even if they already exist");
  init->add_flag("--edit,-e", edit, "Edit the files once they are initialized");

  init->callback([&]() { command = make_unique<Init>(new_files, force, input_files, edit); });


  /*
   * ========================== PROJECT ===============================
   */

  project->add_option("language", language, "The language of the project being created")->required();
  project->add_option("project_name", project_name, "The name of the project being created")->required();

  project->callback([&]() { command = make_unique<Project>(language, project_name); });


  /*
   * ========================== LIB ===============================
   */

  lib->require_subcommand(1);

  auto lib_list   = lib->add_subcommand("list", "List all installed libraries");
  auto lib_create = lib->add_subcommand("create", "Create a library and install it on the system");

  // ========================== LIB LIST ===============================

  lib_list->callback([&]() { command = make_unique<List>(); });

  // ========================== LIB CREATE ===============================

  lib_create->add_option("library_name", pkg_name, "The name of the future library")->required();
  lib_create->add_option("files", input_files, "The headers / binaries of the future library")->required();

  lib_create->add_flag("--force,-f", force, "Force installation even if the library already exists");

  lib_create->callback([&]() { command = make_unique<Create>(pkg_name, input_files, force); });

  /* ========================================================= *
   *                          PARSING                          *
   * ========================================================= */

  // clang-format on
  try
  {
    app.parse(argc, argv);
    if (command)
      return command->execute();
  }
  catch (const CLI::ParseError &e)
  {
    return app.exit(e);
  }
  catch (const ZCError &e)
  {
    cerr << e << endl;
    return e.getCode_();
  }
  catch (const exception &e)
  {
    cerr << RED << "Unexpected error: " << COLOR_RESET << e.what() << endl;
    return -1;
  }
}
