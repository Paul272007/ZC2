#include <vector>

#include <File.hh>
#include <LibCreate.hh>
#include <ZCError.hh>

using namespace std;

LibCreate::LibCreate(const vector<string> &files,
                     const vector<string> &input_files)
{
  for (const auto &file : files)
    files_.push_back(File(file));
}

LibCreate::~LibCreate() {}

int LibCreate::execute()
{
  /*
   * Get languages in the files
   *
   * C library = CHEADER + ARCHIVE || DYNAMIC_LIB
   * Python library = PYC
   * C++ library = CPPHEADER + ARCHIVE || DYNAMIC_LIB
   */
  auto libType = guessLibType();
  switch (libType)
  {
  case C_HEADER:
  case CPP_HEADER:
  case MULTI_LANGUAGES:
    createCLib(libType);
    break;
  default: // Should not happen but we never know
    throw ZCError(7, "Unexpected library type");
    break;
  }
  return 0;
}

Language LibCreate::guessLibType() const
{
  bool found_c_header = false;
  bool found_cpp_header = false;
  bool found_archive = false;
  bool found_dyn = false;
  for (const auto &f : files_)
  {
    switch (f.getLanguage_())
    {
    case C_HEADER:
      found_c_header = true;
      break;
    case CPP_HEADER:
      found_cpp_header = true;
      break;
    case ARCHIVE:
      found_archive = true;
      break;
    case DYNAMIC_LIB:
      found_dyn = true;
      break;
    default:
      throw ZCError(7, "Unexpected file type/language: " + f.getPath_());
      break;
    }
  }
  // No other options right now and since a library can contain C and C++ source
  // files at the same time, we can't make further verifications
  if ((found_archive || found_dyn) && !(found_c_header || found_cpp_header))
    throw ZCError(5, "At least one header file is expected.");
  if (found_c_header && !found_cpp_header)
    return C_HEADER;
  if (found_cpp_header && !found_c_header)
    return CPP_HEADER;
  return MULTI_LANGUAGES;
}

bool LibCreate::createCLib(Language language) const { return true; }
