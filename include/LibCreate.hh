#pragma once

#include <string>

#include <Command.hh>
#include <Config.hh>
#include <File.hh>

class LibCreate : public Command
{
public:
  LibCreate(std::string headerfile, std::string libfile);
  virtual int execute() override;
  ~LibCreate();

private:
  Config config_;
  File headerfile_;
  File archivefile_;
};
