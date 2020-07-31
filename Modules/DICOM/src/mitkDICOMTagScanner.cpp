/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMTagScanner.h"

itk::MutexLock::Pointer mitk::DICOMTagScanner::s_LocaleMutex = itk::MutexLock::New();

mitk::DICOMTagScanner::DICOMTagScanner()
{
}


mitk::DICOMTagScanner::~DICOMTagScanner()
{
}

void mitk::DICOMTagScanner::PushLocale() const
{
  s_LocaleMutex->Lock();

  std::string currentCLocale = setlocale(LC_NUMERIC, nullptr);
  m_ReplacedCLocales.push(currentCLocale);
  setlocale(LC_NUMERIC, "C");

  std::locale currentCinLocale(std::cin.getloc());
  m_ReplacedCinLocales.push(currentCinLocale);
  std::locale l("C");
  std::cin.imbue(l);

  s_LocaleMutex->Unlock();
}

void mitk::DICOMTagScanner::PopLocale() const
{
  s_LocaleMutex->Lock();

  if (!m_ReplacedCLocales.empty())
  {
    setlocale(LC_NUMERIC, m_ReplacedCLocales.top().c_str());
    m_ReplacedCLocales.pop();
  }
  else
  {
    MITK_WARN << "Mismatched PopLocale on DICOMITKSeriesGDCMReader.";
  }

  if (!m_ReplacedCinLocales.empty())
  {
    std::cin.imbue(m_ReplacedCinLocales.top());
    m_ReplacedCinLocales.pop();
  }
  else
  {
    MITK_WARN << "Mismatched PopLocale on DICOMITKSeriesGDCMReader.";
  }

  s_LocaleMutex->Unlock();
}

std::string mitk::DICOMTagScanner::GetActiveLocale()
{
  return setlocale(LC_NUMERIC, nullptr);
}
