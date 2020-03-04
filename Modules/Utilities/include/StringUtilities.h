#pragma once

#include <string>

#include <MitkUtilitiesExports.h>

namespace Utilities
{
  MITKUTILITIES_EXPORT std::string convertLocalToUTF8(const std::string& str);
  MITKUTILITIES_EXPORT std::string convertLocalToOEM(const std::string& str);
  MITKUTILITIES_EXPORT std::string convertUTF8ToLocal(const std::string& str);

#ifdef _WIN32
  MITKUTILITIES_EXPORT std::string convertToUtf8(const std::wstring& wstr);
#endif

  MITKUTILITIES_EXPORT bool isValidUtf8(const char* str);

  /// Convert text from DICOM Person Name form to readable
  /// \param pn a tag source value in the PN form (<Alphabetic representation,>[=<Ideographic representation>[=<Phonetic representation>]).
  ///   Where each representation consists of up to 5 name components (Last Name(s)[^First Name(s)[^Middle Name(s)[^Prefix[^Postfix]]]]).
  /// \param group index of component group (0..2) to be extracted.
  /// \return the specified group in readable form or the source string if the conversion fails.
  /// UTF-8 is assumed for input and output strings.
  MITKUTILITIES_EXPORT std::string formatPersonName(const std::string& pn, int group=0);
}
