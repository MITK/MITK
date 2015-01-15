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

struct MITK_CORE_EXPORT LocaleSwitch
{
  LocaleSwitch(const std::string& newLocale)
    : m_OldLocale(std::setlocale(LC_ALL, NULL))
    , m_NewLocale(newLocale)
  {
    if (m_OldLocale == NULL)
    {
      m_OldLocale = "";
    }
    else if (m_NewLocale != m_OldLocale)
    {
      // set the locale
      if (std::setlocale(LC_ALL, m_NewLocale.c_str()) == NULL)
      {
        MITK_INFO << "Could not set locale " << m_NewLocale;
        m_OldLocale = NULL;
      }
    }
  }
  ~LocaleSwitch()
  {
    if (m_OldLocale != NULL && std::setlocale(LC_ALL, m_OldLocale) == NULL)
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
