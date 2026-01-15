#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <CLI11.hpp>
#include <Init.hh>
#include <LibCreate.hh>
#include <LibList.hh>
#include <Run.hh>
#include <ZCError.hh>

using namespace std;

int main(int argc, char *argv[])
{
  /* Create CLI11 app */
  CLI::App app{"ZC compiler, the easiest way to compile, execute and test "
               "C/C++ programs"};
  argv = app.ensure_utf8(argv);
  app.set_version_flag("--version,-v", "ZC v0.0.1 (dev)");

  app.require_subcommand(1);
  // ==================== Variables to store arguments
  // run
  bool run_keep = false;
  bool run_plus = false;
  bool run_c = false;
  bool run_S = false;
  bool run_E = false;
  vector<string> run_args;

  // lib (empty for the moment)
  string lib_output_name;
  // init
  string init_file, init_output;
  bool init_edit = false;

  // Variables for all commands
  vector<string> files;
  vector<string> input_files;
  bool force = false;
  string library_name;

  /* Store the object that is going to execute the command */
  unique_ptr<Command> current_command(nullptr);

  /*
   * ===============================================
   * =============================================== Subcommands
   * ===============================================
   */

  // ========================= RUN subcommand =========================

  auto sub_run = app.add_subcommand("run", "Simply compile and run C/C++ file");
  sub_run
      ->add_option("files", input_files,
                   "The files to be compiled and executed")
      ->required();
  sub_run->add_flag("--keep,-k", run_keep,
                    "Do not delete the executable after being executed");
  sub_run->add_flag("--plus,-p", run_plus, "Force compilation as C++");
  sub_run->add_flag("-c", run_c, "Compile and assemble, but do not link");
  sub_run->add_flag("-S", run_S, "Compile only");
  sub_run->add_flag("-E", run_E, "Preprocess only");
  sub_run->add_option("--args,-a", run_args,
                      "Arguments to be passed to the program");
  sub_run->callback(
      [&]()
      {
        current_command = make_unique<Run>(input_files, run_args, run_keep,
                                           run_plus, run_E, run_S, run_c);
      });

  // ========================= LIB subcommand =========================

  auto sub_lib = app.add_subcommand("lib", "Actions on libraries.");
  sub_lib->require_subcommand(1);

  // lib list
  sub_lib->add_subcommand("list", "List all installed libraries")
      ->callback([&]() { current_command = make_unique<LibList>(); });

  // -----------------------------------------------------------------------------

  // lib create

  auto sub_lib_create = sub_lib->add_subcommand(
      "create", "Create a static (.a) or shared (.so) library");

  sub_lib_create
      ->add_option("output", lib_output_name,
                   "Output file (e.g. libmylib.a or libmylib.so)")
      ->required();

  sub_lib_create
      ->add_option("files", files, "Source files (.c, .cpp) or objects (.o)")
      ->required();

  sub_lib_create->add_flag(
      "--force,-f", force,
      "Force writing even the files already exist in the zc directory");

  sub_lib_create->callback(
      [&]()
      {
        current_command = make_unique<LibCreate>(lib_output_name, files, force);
      });

  // ========================= INIT subcommand =========================

  auto sub_init = app.add_subcommand(
      "init", "Create and initiliaze a file based on its extension.");
  sub_init->add_option("file", init_file, "Output file")->required();
  sub_init->add_flag("--force,-f", force);
  // sub_init->add_option("--output,-o", init_output, "Specify output file");
  sub_init->add_option("--input,-i", input_files,
                       "Source code to be used as basis for the new file");
  sub_init->add_flag("--edit,-e", init_edit);
  sub_init->callback(
      [&]()
      {
        current_command =
            make_unique<Init>(init_file, force, input_files, init_edit);
      });

  // ========================= Parsing =========================
  try
  {
    app.parse(argc, argv);
    if (current_command)
      return current_command->execute();
  }
  catch (const CLI::ParseError &e) // Handle CLI11 errors first
  {
    return app.exit(e);
  }
  catch (const ZCError &e) // Write a formatted message for normal ZCErrors
  {
    if (e.code() != 6)
      cerr << e << endl;
    else
      cerr << "Operation canceled." << endl;
    return e.code();
  }
  catch (const exception &e) // Write a special message for unexpected errors
  {
    cerr << "Unexpected error: " << e.what() << endl;
    return -1;
  }
}
