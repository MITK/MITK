/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLocaleSwitch.h"

#include "mitkLog.h"

#include <clocale>
#include <string>

namespace mitk
{
  struct LocaleSwitch::Impl
  {
    explicit Impl(const std::string &newLocale);

    ~Impl();

  private:
    /// locale at instantiation of object
    std::string m_OldLocale;

    /// locale during life-time of object
    const std::string m_NewLocale;
  };

  LocaleSwitch::Impl::Impl(const std::string &newLocale) : m_NewLocale(newLocale)
  {
    // query and keep the current locale
    const char *currentLocale = std::setlocale(LC_ALL, nullptr);
    if (currentLocale != nullptr)
      m_OldLocale = currentLocale;
    else
      m_OldLocale = "";

    // install the new locale if it different from the current one
    if (m_NewLocale != m_OldLocale)
    {
      if (!std::setlocale(LC_ALL, m_NewLocale.c_str()))
      {
        MITK_INFO << "Could not switch to locale " << m_NewLocale;
        m_OldLocale = "";
      }
    }
  }

  LocaleSwitch::Impl::~Impl()
  {
    if (!m_OldLocale.empty() && m_OldLocale != m_NewLocale && !std::setlocale(LC_ALL, m_OldLocale.c_str()))
    {
      MITK_INFO << "Could not reset original locale " << m_OldLocale;
    }
  }

  LocaleSwitch::LocaleSwitch(const char *newLocale) : m_LocaleSwitchImpl(new Impl(newLocale)) {}
  LocaleSwitch::~LocaleSwitch() { delete m_LocaleSwitchImpl; }
}
