#ifndef RUN_H_
#define RUN_H_

#include <Command.hh>
#include <string>
#include <vector>

class Run : public Command
{
public:
  Run(const std::vector<std::string> &files,
      const std::vector<std::string> &args, bool keep, bool plus,
      bool preprocess, bool compile, bool assemble);
  virtual int execute() override;
  virtual ~Run();

private:
  std::vector<std::string> files_;
  bool keep_;
  bool plus_;
  bool preprocess_;
  bool compile_;
  bool assemble_;
  std::vector<std::string> args_;
};

#endif // !RUN_H_
