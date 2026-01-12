#ifndef RUN_H_
#define RUN_H_

#include <string>
#include <vector>

#include <Command.hh>
#include <Config.hh>
#include <File.hh>

enum CompilingMode
{
  FULL,
  PREPROCESS,
  COMPILE,
  ASSEMBLE
};

using Mode = enum CompilingMode;

class Run : public Command
{
public:
  Run(const std::vector<std::string> &files,
      const std::vector<std::string> &args, bool keep, bool plus,
      bool preprocess, bool compile, bool assemble);
  virtual int execute() override;
  virtual ~Run();

private:
  /**
   * @brief Get the compiling mode based on the command line arguments
   */
  Mode getMode(bool preprocess, bool compile, bool assemble) const;
  /**
   * @brief Find the main file from a list of files.
   * @return The main file path without extension
   */
  std::string findMainFile() const;
  /**
   * @brief know if we have to compile as C++
   * @return true if the files contain at least one C++ file
   */
  bool isCpp() const;
  /**
   * @brief Build a cmd for the compiler and store it in buildCmd
   */
  void buildCommand(std::string &buildCmd,
                    const std::string &output_name) const;
  /**
   * @brief Checks if all files do exist
   * @return false when at least one file doesn't exist, true otherwise
   */
  bool filesExist(std::string &fileNotFound) const;
  /**
   * @brief Checks if all files have the correct extension for C/C++ compiling
   * @return false when at least one file doesn't have the correct extension,
   * true otherwise
   */
  bool filesHaveCorrectExt(std::string &badFile) const;
  std::vector<std::string> getInclusions() const;
  /**
   * @brief true if we want to keep the executable, false otherwise
   */
  bool keep_;
  /**
   * @brief true if we want to force compiling as C++
   */
  bool plus_;
  /**
   * @brief Compilation mode : see enum declaration for more info
   */
  Mode mode_;
  /**
   * @brief Contains the configuration of ZC
   */
  Config config_;
  /**
   * @brief The arguments to be passed to the program when executing it
   */
  std::vector<std::string> args_;
  /**
   * @brief The list containing files passed to the command
   */
  std::vector<File> files_;
};

#endif // !RUN_H_
