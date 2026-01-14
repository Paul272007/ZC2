#ifndef LIB_H_
#define LIB_H_

// #include <string>
// #include <vector>

#include <Command.hh>
#include <Config.hh>

// int lib_list();
// int lib_install(std::string h, std::string a, bool force);
// int lib_create(std::vector<std::string> files, bool force);
// int lib_remove(std::string lib);

class LibList : public Command
{
public:
  LibList();
  virtual int execute() override;
  virtual ~LibList();

private:
  Config config_;
};

#endif // !LIB_H_
