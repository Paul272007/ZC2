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
  Init(const std::vector<std::string> &files, bool force,
       std::vector<std::string> &input_files, bool edit);
  virtual int execute() override;

private:
  std::vector<File> getTemplates();
  bool writeCDecls(const File &f) const;
  bool force_;
  bool edit_;
  Settings &settings_;
  std::vector<File> files_;
  std::vector<File> input_files_;
  std::filesystem::path templates_path_ = getZCRootDir() / TEMPLATES;
};
