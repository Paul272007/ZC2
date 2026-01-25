#include <commands/Project.hh>

using namespace std;

Project::Project(const string &language, const string &project_name)
    : language_(language), project_name_(project_name)
{
}

int Project::execute() { return 0; }
