#pragma once

#include <filesystem>
#include <string>
#include <vector>

class Registry;

enum Language
{
  C,
  CPP,
  H,
  HPP,
  PY,
  PYC,
  ARCHIVE,
  DYN_LIB,
  OBJECT,
  INSTANCE,
  ASSEMBLER,
  MULTI_LANGUAGES,
  OTHER
};

class File
{
public:
  File(const std::string &path);
  bool write(const std::string &content);
  std::string read() const;
  bool copy(const File &file);
  bool exists() const;
  bool remove() const;

  /**
   * @brief Get inclusions from file and return associated compiling flags
   */
  std::vector<std::string> getInclusions(const Registry &reg) const;

  /* Getters and setters */
  std::string getPath_() const;
  std::string getFilename() const;
  Language getLanguage_() const;

private:
  std::filesystem::path path_;
  std::string filename_;
  Language language_;
};

std::ostream &operator<<(std::ostream &stream, const File &file);
