#include <Run.hh>

using namespace std;

Run::Run(const std::vector<std::string> &files,
         const std::vector<std::string> &args, bool keep, bool plus,
         bool preprocess, bool compile, bool assemble)
    : files_(files), args_(args), keep_(keep), plus_(plus),
      preprocess_(preprocess), compile_(compile), assemble_(assemble)
{
}
