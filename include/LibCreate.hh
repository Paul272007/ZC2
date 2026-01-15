#pragma once

#include <string>
#include <vector>

#include <Command.hh>
#include <Config.hh>
#include <File.hh>

class LibCreate : public Command
{
public:
  LibCreate(const std::string &output_name,
            const std::vector<std::string> &files, bool force);
  virtual int execute() override;
  ~LibCreate();

private:
  /**
   * @brief Create and index a library based on the given files
   *
   * @param language Quite counter-intuitive but for the index in the config
   * file, it goes in the category correspond to its type : C, C++ or multi
   * languages, but the procedure is the same so they go into the same function
   * @return true if the installation was successful, false otherwise
   */
  bool isShared() const;
  void compileObjects(const std::vector<std::string> &sources,
                      std::vector<std::string> &objects, bool shared) const;
  // bool createCLib(Language language);
  // Language guessLibType() const;
  // bool addFileToHeaders(const File &f);
  // bool addFileToBinaries(const File &f);
  //
  // std::vector<File> files_;
  // std::vector<File> headers_;
  // std::vector<File> binaries_;
  bool force_;
  Config config_;
  std::string output_name_;
  std::vector<std::string> files_;
};
