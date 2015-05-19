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



#ifndef MITKLOCALESWITCH_H_
#define MITKLOCALESWITCH_H_

#include <MitkCoreExports.h>
#include <clocale>

namespace mitk {

struct MITKCORE_EXPORT LocaleSwitch
{
  LocaleSwitch(const std::string& newLocale)
    : m_OldLocale(std::setlocale(LC_ALL, nullptr))
    , m_NewLocale(newLocale)
  {
    if (m_OldLocale == nullptr)
    {
      m_OldLocale = "";
    }
    else if (m_NewLocale != m_OldLocale)
    {
      // set the locale
      if (std::setlocale(LC_ALL, m_NewLocale.c_str()) == nullptr)
      {
        MITK_INFO << "Could not set locale " << m_NewLocale;
        m_OldLocale = nullptr;
      }
    }
  }
  ~LocaleSwitch()
  {
    if (m_OldLocale != nullptr && std::setlocale(LC_ALL, m_OldLocale) == nullptr)
    {
      MITK_INFO << "Could not reset locale " << m_OldLocale;
    }
  }
private:
  const char* m_OldLocale;
  const std::string m_NewLocale;
};
}

#endif // MITKLOCALESWITCH_H_
