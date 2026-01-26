#include <filesystem>
#include <string>
#include <vector>

#include <commands/Command.hh>
#include <helpers.hh>
#include <objects/File.hh>
#include <objects/Settings.hh>

#define TEMPLATES "templates"

class Init : public Command
{
public:
  /**
   * @brief Create an instance of the Init command
   *
   * @param files The files to be initialized
   * @param force Force creation even if the files already exist
   * @param input_files Files to be used as basis to create the new ones
   * @param edit Edit file after creating it
   */
  Init(const std::vector<std::string> &files, bool force,
       std::vector<std::string> &input_files, bool edit);

  /**
   * @brief Execute function
   *
   * @return return code depending on whether or not the function ended
   * successfully
   */
  virtual int execute() override;

private:
  /**
   * @brief Return all the templates that exist for the specific file type
   */
  std::vector<File> getTemplates();

  /**
   * @brief Write the C declarations into the header file
   *
   * @param f The file to be written
   * @return Whether or not the file was written successfully
   */
  bool writeCDecls(const File &f) const;

  bool force_;
  bool edit_;
  Settings &settings_;
  std::vector<File> files_;
  std::vector<File> input_files_;
  std::filesystem::path templates_path_ = getZCRootDir() / TEMPLATES;
};
