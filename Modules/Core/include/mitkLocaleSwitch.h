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

#include <clocale>

namespace mitk
{

/**
  \brief Convenience class to temporarily change the current locale.

  This helper class can be used to switch to a specific locale
  for a couple of operations. Once the class is destroyed, the previous
  locale will be restored. This avoid calling or forgetting to call
  setlocale() in multiple return locations.

  Typically this is used to switch to a "C" locale when parsing or
  printing numbers, in order to consistently get "." and not "," as
  a decimal separator.

  \code

  std::string toString(int number)
  {
    mitk::LocaleSwitch("C"); // installs "C" locale until end of function

    std::stringstream parser;
    parser << number;

    return parser.str();
  }
  \endcode
*/
struct MITKCORE_EXPORT LocaleSwitch
{
  LocaleSwitch(const std::string& newLocale)
  : m_NewLocale( newLocale )
  {
    // query and keep the current locale
    const char* currentLocale = std::setlocale(LC_ALL, nullptr);
    if (currentLocale != nullptr)
        m_OldLocale = currentLocale;
    else
        m_OldLocale = "";

    // install the new locale if it different from the current one
    if (m_NewLocale != m_OldLocale)
    {
      if ( ! std::setlocale(LC_ALL, m_NewLocale.c_str()) )
      {
        MITK_INFO << "Could not switch to locale " << m_NewLocale;
        m_OldLocale = "";
      }
    }
  }

  ~LocaleSwitch()
  {
    if ( !m_OldLocale.empty() && m_OldLocale != m_NewLocale && !std::setlocale(LC_ALL, m_OldLocale.c_str()) )
    {
      MITK_INFO << "Could not reset original locale " << m_OldLocale;
    }
  }

private:

  /// locale at instantiation of object
  std::string m_OldLocale;

  /// locale during life-time of object
  const std::string m_NewLocale;
};

}

#endif // __mitkLocaleSwitch_h
