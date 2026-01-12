#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <CLI11.hpp>
#include <Header.hh>
#include <Lib.hh>
#include <Run.hh>
#include <ZCError.hh>
// namespace fs = std::filesystem;

using namespace std;

int main(int argc, char *argv[])
{
  CLI::App app{"ZC compiler, the easiest way to compile, execute and test "
               "C/C++ programs"};
  argv = app.ensure_utf8(argv);
  app.set_version_flag("--version,-v", "ZC v0.0.1 (dev)");

  app.require_subcommand(1);
  // ==================== Variables to store arguments
  // run
  vector<string> run_files;
  bool run_keep = false;
  bool run_plus = false;
  bool run_c = false;
  bool run_S = false;
  bool run_E = false;
  vector<string> run_args;
  // lib
  string lib_header, lib_archive, lib_remove_target;
  vector<string> lib_create_files;
  bool lib_force = false;
  // header
  vector<string> head_create_files;
  string head_output, head_sync_h, head_sync_c, head_init_target;
  bool head_force = false;

  unique_ptr<Command> current_command(nullptr);

  // ========================= RUN subcommand =========================
  auto sub_run =
      app.add_subcommand("run", "Simply compile and run C/C++ file.");
  sub_run
      ->add_option("files", run_files, "The files to be compiled and executed")
      ->required();
  sub_run->add_flag("--keep,-k", run_keep,
                    "Do not delete the executable after being executed.");
  sub_run->add_flag("--plus,-p", run_plus, "Force compilation as C++");
  sub_run->add_flag("-c", run_c, "Compile and assemble, but do not link.");
  sub_run->add_flag("-S", run_S, "Compile only");
  sub_run->add_flag("-E", run_E, "Preprocess only");
  sub_run->add_option("--args,-a", run_args,
                      "Arguments to be passed to the program.");
  sub_run->callback(
      [&]()
      {
        current_command = make_unique<Run>(run_files, run_args, run_keep,
                                           run_plus, run_E, run_S, run_c);
      });

  // ========================= LIB subcommand =========================
  auto sub_lib = app.add_subcommand("lib", "Actions on libraries.");
  sub_lib->require_subcommand(1);

  // lib list
  sub_lib->add_subcommand("list", "List all installed libraries")
      ->callback([&]() { current_command = make_unique<Lib>(); });

  // lib install
  auto sub_lib_install =
      sub_lib->add_subcommand("install", "Install a new library");
  sub_lib_install->add_option("headerfile", lib_header, "The header file (.h)")
      ->required();
  sub_lib_install->add_option("libfile", lib_archive, "The archive file (.a)")
      ->required();
  sub_lib_install->add_flag("--force,-f", lib_force, "Force writing");
  sub_lib_install->callback([&]() { current_command = make_unique<Lib>(); });

  // lib create
  auto sub_lib_create =
      sub_lib->add_subcommand("create", "Create and install a static library");
  sub_lib_create
      ->add_option("files", lib_create_files, "The source code files (.c)")
      ->required();
  sub_lib_create->add_flag("--force,-f", lib_force, "Force writing");
  sub_lib_create->callback([&]() { current_command = make_unique<Lib>(); });

  // lib remove
  auto sub_lib_remove = sub_lib->add_subcommand("remove", "Remove a library");
  sub_lib_remove
      ->add_option("library", lib_remove_target, "The library header to remove")
      ->required();
  sub_lib_remove->callback([&]() { current_command = make_unique<Lib>(); });

  // ========================= HEADER subcommand =========================
  auto sub_header = app.add_subcommand("header", "Actions on header files.");
  sub_header->require_subcommand(1);

  // header create
  auto sub_head_create =
      sub_header->add_subcommand("create", "Generate a new header file");
  sub_head_create->add_option("files", head_create_files, "The C files")
      ->required();
  sub_head_create->add_flag("--force,-f", head_force);
  sub_head_create->add_option("--output,-o", head_output,
                              "Specify output file");
  sub_head_create->callback([&]() { current_command = make_unique<Header>(); });

  // header sync
  auto sub_head_sync =
      sub_header->add_subcommand("sync", "Modify existing header file");
  sub_head_sync->add_option("headerfile", head_sync_h)->required();
  sub_head_sync->add_option("cfile", head_sync_c)->required();
  sub_head_sync->callback([&]() { current_command = make_unique<Header>(); });

  // header init
  auto sub_head_init = sub_header->add_subcommand("init", "Init a header file");
  sub_head_init->add_option("headerfile", head_init_target)->required();
  sub_head_init->add_flag("--force,-f", head_force);
  sub_head_init->callback([&]() { current_command = make_unique<Header>(); });

  // ========================= Parsing =========================
  try
  {
    app.parse(argc, argv);
  }
  catch (const CLI::ParseError &e)
  {
    return app.exit(e);
  }
  try
  {
    if (current_command)
      return current_command->execute();
  }
  catch (const ZCError &e)
  {
    if (e.code() != 6)
      cerr << e << endl;
    else
      cerr << "Operation canceled." << endl;
    return e.code();
  }
  catch (const exception &e)
  {
    cerr << "Unexpected error: " << e.what();
    return -1;
  }
}
