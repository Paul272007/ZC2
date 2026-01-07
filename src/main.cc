#include <CLI11.hpp>
#include <core.hpp>
#include <filesystem>
#include <header.hpp>
#include <iostream>
#include <lib.hpp>
#include <run.hpp>
#include <string>
#include <vector>
#include <zcerror.hpp>

namespace fs = std::filesystem;

using namespace std;

// Valideur d'extension
struct CheckExt : public CLI::Validator
{
  CheckExt(vector<string> exts) : Validator("ExtensionValidator")
  {
    func_ = [exts](string &filename)
    {
      string actual_ext = fs::path(filename).extension().string();
      for (const auto &ext : exts)
      {
        if (actual_ext == ext)
          return string(); // OK
      }
      return "File must have one of these extensions: " +
             CLI::detail::join(exts);
    };
  }
};

// Valideur d'existence ET d'extension
CLI::Validator FileExistAndExt(vector<string> exts)
{
  return CLI::ExistingFile & CheckExt(exts);
}

int main(int argc, char *argv[])
{
  CLI::App app{"ZC compiler, the easiest way to compile, execute and test "
               "C/C++ programs using GCC"};
  argv = app.ensure_utf8(argv);
  app.set_version_flag("--version,-v", "ZC v0.0.1 (dev)");

  app.require_subcommand(1);
  // ==================== Variables to store arguments
  // run
  vector<string> run_files;
  bool run_keep = false;
  bool run_plus = false;
  bool run_c = false;
  vector<string> run_args;
  // lib
  string lib_header, lib_archive, lib_remove_target;
  vector<string> lib_create_files;
  bool lib_force = false;
  // header
  vector<string> head_create_files;
  string head_output, head_sync_h, head_sync_c, head_init_target;
  bool head_force = false;

  auto exist_cs = FileExistAndExt({".c", ".o", ".cpp", ".cxx", ".cc"});
  auto exist_h = FileExistAndExt({".h", ".hpp"});
  auto exist_a = FileExistAndExt({".a"});
  auto exist_c = FileExistAndExt({".c"});
  auto valid_h = CheckExt({".h", ".hpp"});

  // ========================= RUN subcommand =========================
  auto sub_run =
      app.add_subcommand("run", "Simply compile and run C/C++ file.");
  sub_run
      ->add_option("files", run_files, "The files to be compiled and executed")
      ->check(exist_cs)
      ->required();
  sub_run->add_flag("--keep,-k", run_keep,
                    "Do not delete the executable after being executed.");
  sub_run->add_flag("--plus,-p", run_plus, "Force compilation as C++");
  sub_run->add_flag("-c", run_c, "Compile and assemble, but do not link.");
  sub_run->add_option("--args,-a", run_args,
                      "Arguments to be passed to the program.");
  sub_run->callback(
      [&]() { run_func(run_files, run_keep, run_plus, run_c, run_args); });

  // ========================= LIB subcommand =========================
  auto sub_lib = app.add_subcommand("lib", "Actions on libraries.");
  sub_lib->require_subcommand(1);

  // lib list
  sub_lib->add_subcommand("list", "List all installed libraries")
      ->callback([&]() { lib_list(); });

  // lib install
  auto sub_lib_install =
      sub_lib->add_subcommand("install", "Install a new library");
  sub_lib_install->add_option("headerfile", lib_header, "The header file (.h)")
      ->check(exist_h)
      ->required();
  sub_lib_install->add_option("libfile", lib_archive, "The archive file (.a)")
      ->check(exist_a)
      ->required();
  sub_lib_install->add_flag("--force,-f", lib_force, "Force writing");
  sub_lib_install->callback(
      [&]() { lib_install(lib_header, lib_archive, lib_force); });

  // lib create
  auto sub_lib_create =
      sub_lib->add_subcommand("create", "Create and install a static library");
  sub_lib_create
      ->add_option("files", lib_create_files, "The source code files (.c)")
      ->check(exist_c)
      ->required();
  sub_lib_create->add_flag("--force,-f", lib_force, "Force writing");
  sub_lib_create->callback([&]() { lib_create(lib_create_files, lib_force); });

  // lib remove
  auto sub_lib_remove = sub_lib->add_subcommand("remove", "Remove a library");
  sub_lib_remove
      ->add_option("library", lib_remove_target, "The library header to remove")
      ->check(valid_h)
      ->required();
  sub_lib_remove->callback([&]() { lib_remove(lib_remove_target); });

  // ========================= HEADER subcommand =========================
  auto sub_header = app.add_subcommand("header", "Actions on header files.");
  sub_header->require_subcommand(1);

  // header create
  auto sub_head_create =
      sub_header->add_subcommand("create", "Generate a new header file");
  sub_head_create->add_option("files", head_create_files, "The C files")
      ->check(exist_c)
      ->required();
  sub_head_create->add_flag("--force,-f", head_force);
  sub_head_create->add_option("--output,-o", head_output,
                              "Specify output file");
  sub_head_create->callback(
      [&]() { header_create(head_create_files, head_force, head_output); });

  // header sync
  auto sub_head_sync =
      sub_header->add_subcommand("sync", "Modify existing header file");
  sub_head_sync->add_option("headerfile", head_sync_h)
      ->check(exist_h)
      ->required();
  sub_head_sync->add_option("cfile", head_sync_c)->check(exist_c)->required();
  sub_head_sync->callback(
      [&]() { header_sync(head_sync_h, head_sync_c, head_output); });

  // header init
  auto sub_head_init = sub_header->add_subcommand("init", "Init a header file");
  sub_head_init->add_option("headerfile", head_init_target)
      ->check(valid_h)
      ->required();
  sub_head_init->add_flag("--force,-f", head_force);
  sub_head_init->callback(
      [&]() { header_init(head_init_target, head_force, head_output); });

  // ========================= Parsing =========================
  try
  {
    CLI11_PARSE(app, argc, argv);
  }
  catch (const ZCError &e)
  {
    cerr << e << endl;
    return e.code();
  }
  return 0;
}
