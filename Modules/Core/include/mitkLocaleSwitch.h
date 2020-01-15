/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __mitkLocaleSwitch_h
#define __mitkLocaleSwitch_h

#include "MitkCoreExports.h"

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
    explicit LocaleSwitch(const char *newLocale);

    ~LocaleSwitch();

    LocaleSwitch(LocaleSwitch &) = delete;
    LocaleSwitch operator=(LocaleSwitch &) = delete;

  private:
    struct Impl;
    Impl *m_LocaleSwitchImpl;
  };
}

#endif // __mitkLocaleSwitch_h
