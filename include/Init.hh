#ifndef HEADER_H_
#define HEADER_H_

#include <functional>
#include <string>
#include <vector>

#include <Command.hh>
#include <Config.hh>
#include <File.hh>

class Init : public Command
{
public:
  using Action = std::function<int()>;

  Init(std::string &output_file, bool force,
       std::vector<std::string> &input_files, bool edit);
  virtual int execute() override;
  virtual ~Init();

private:
  int writeCDecls() const;

  /**
   * @brief A helper to write into the files
   */
  bool write() const;

  /**
   * @brief The output file that is written into
   */
  File output_file_;

  /**
   * @brief True if the input_files_ array is empty, false otherwise
   */
  bool has_input_;

  /**
   * @brief Whether to force writing into the output file even if it already
   * exists or not
   */
  bool force_;

  /**
   * @brief Whether or not to enter an editor after the file being created to
   * edit it
   */
  bool edit_;

  /**
   * @brief Contains the configuration of ZC
   */
  Config config_;

  /**
   * @brief The input files used to fill the output file
   */
  std::vector<File> input_files_;
};

#endif // !HEADER_H_
