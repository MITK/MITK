/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIFileIO.h>

#include <mitkExceptionMacro.h>

#include <vector>

namespace mitk
{
  IFileIO::~IFileIO() {}
  std::string mitk::IFileIO::PROP_DESCRIPTION()
  {
    static std::string s = "org.mitk.IFileIO.description";
    return s;
  }

  std::string mitk::IFileIO::PROP_MIMETYPE()
  {
    static std::string s = "org.mitk.IFileIO.mimetype";
    return s;
  }

  std::string IFileIO::ResolveEnumOption(const Options &options, const std::string &name)
  {
    const auto finding = options.find(name);
    if (finding == options.end() || finding->second.Empty())
      mitkThrow() << "The option \"" << name << "\" is not set.";

    const auto &value = finding->second;
    if (value.Type() == typeid(std::string))
      return us::any_cast<std::string>(value);

    if (value.Type() == typeid(std::vector<std::string>))
    {
      const auto &choices = us::ref_any_cast<std::vector<std::string>>(value);
      if (choices.empty())
        mitkThrow() << "The option \"" << name << "\" offers no choices.";
      return choices.front();
    }

    mitkThrow() << "The option \"" << name << "\" holds a value of type " << value.Type().name()
                << " instead of a string or a list of strings.";
  }
}
