/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLocaleSwitch_h
#define mitkLocaleSwitch_h

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

    WARNING: Please be aware that using setlocale and there for is not thread
    safe. So use this class with care (see task T24295 for more information.
    This switch is especially use full if you have to deal with third party code
    where you have to control the locale via set locale
    \code
    {
      mitk::LocaleSwitch localeSwitch("C");// installs C locale until the end of the function

      ExternalLibraryCall(); //that might throw or what ever.
    }
    \endcode

    If you just want to control you own stringstream operations use imbue instead, as it is
    threadsafe. E.G.:
    \code
    std::string toString(int number)
    {
      std::ostringstream parser;
      parser.imbue(std::locale("C"));
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

#endif
