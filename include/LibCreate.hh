#pragma once

#include <string>

#include <Command.hh>
#include <Config.hh>
#include <File.hh>
#include <vector>

class LibCreate : public Command
{
public:
  LibCreate(const std::vector<std::string> &files,
            const std::vector<std::string> &input_files);
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
  bool createCLib(Language language) const;
  Language guessLibType() const;

  Config config_;
  std::vector<File> files_;
};
