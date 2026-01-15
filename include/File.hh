#ifndef FILE_H_
#define FILE_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <Config.hh>

using Declarations = std::map<std::string, std::vector<std::string>>;

enum Language
{
  C,               // C source file (.c)
  CPP,             // C++ source file (.cpp, .cc, .cxx)
  C_HEADER,        // C header file (.h)
  CPP_HEADER,      // C++ header file (.hpp, .hh, .hxx)
  PYTHON,          // Python file (.py)
  ARCHIVE,         // Archive file / static library (.a)
  DYNAMIC_LIB,     // Dynamic library (.so)
  PYC,             // Compressed python file (.pyc)
  MULTI_LANGUAGES, // More than one language (not useful for single files)
  UNSUPPORTED      // Unsupported file type
};

class File
{
public:
  File(std::string filepath);
  ~File();

  /**
   * @brief Get the file path
   *
   * @return path_
   */
  std::string getPath_() const;

  /**
   * @brief Get the file's name instead of its complete path
   */
  std::string getFilename() const;

  /**
   * @brief Get the file language, based on its extension
   *
   * @return the language of the file
   */
  Language getLanguage_() const;

  /**
   * @brief Get the file extension
   */
  std::string getExt() const;

  /**
   * @brief Get all declarations from a C file
   * All declarations such as functions, enums, structs, includes, macros,
   * typedefs, unions, and global variables
   */
  std::unique_ptr<Declarations> parse() const;

  /**
   * @brief Check if the file contains a main function.
   */
  bool containsMain() const;

  /**
   * @brief Get all #include in the file
   */
  std::vector<std::string>
  getInclusions(const std::vector<Library> &libs) const;

  /**
   * @brief Write content to the file.
   */
  bool writeContent(const std::string &content) const;

  /**
   * @brief Append content at the end of the file
   */
  bool appendContent(const std::string &content) const;

  /**
   * @brief Check if the file exists.
   */
  bool exists() const;

  /**
   * @brief Write the formatted C declarations to the file.
   */
  bool writeDeclarations(const Declarations &decls) const;

  /**
   * @brief Initialize a new python file
   */
  bool initPython() const;

  /**
   * @brief Initialize a new C source file
   */
  bool initC() const;

  /**
   * @brief Initialize a new C++ source file
   */
  bool initCPP() const;

  /**
   * @brief Initialize a new C header file
   */
  bool initH() const;

  /**
   * @brief Initialize a new C++ header file
   *
   * @return true if it was successful, false otherwise
   */
  bool initHPP() const;

  /**
   * @brief Display the filepath to f
   */
  void display(std::ostream &f = std::cout) const;

  /**
   * @brief Get the constant corresponding to this file to make header guards
   *
   * @return FILENAME_H_
   */
  std::string getHeaderGuardConstant() const;

  /**
   * @brief Copy the file content to f
   *
   * @param f the file that gets filled
   * @return true if the operation was successful, false otherwise
   */
  bool copyTo(File &f) const;

private:
  std::string path_;

  Language language_;

  /**
   * @brief Reads the content of the file.
   */
  std::string readFile() const;
};

std::ostream &operator<<(std::ostream &s, const File &f);

#endif // !FILE_H_
