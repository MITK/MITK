/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef __mitkLocaleSwitch_h
#define __mitkLocaleSwitch_h

#include "MitkCoreExports.h"
#include "mitkLogMacros.h"

#include <clocale>
#include <string>

#pragma warning(disable:4251)

namespace mitk
{

/**
  \brief Convenience class to temporarily change the current locale.

  This helper class can be used to switch to a specific locale
  for a couple of operations. Once the class is destroyed, the previous
  locale will be restored. This avoids calling or forgetting to call
  setlocale() in multiple return locations.

  Typically this is used to switch to a "C" locale when parsing or
  printing numbers, in order to consistently get "." and not "," as
  a decimal separator.

  \code

  std::string toString(int number)
  {
    mitk::LocaleSwitch localeSwitch("C");// installs C locale until the end of the function

    std::stringstream parser;
    parser << number;

    return parser.str();
  }
  \endcode
*/

struct MITKCORE_EXPORT LocaleSwitch
{
  explicit LocaleSwitch(const char* newLocale);

  ~LocaleSwitch();

  LocaleSwitch(LocaleSwitch&) = delete;
  LocaleSwitch operator=(LocaleSwitch&) = delete;
private:

  struct Impl;
  Impl* m_LocaleSwitchImpl;
};

}

#endif // __mitkLocaleSwitch_h
