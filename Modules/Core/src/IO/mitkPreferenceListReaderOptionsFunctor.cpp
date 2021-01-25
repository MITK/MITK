/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPreferenceListReaderOptionsFunctor.h"

mitk::PreferenceListReaderOptionsFunctor::PreferenceListReaderOptionsFunctor()
{

}

mitk::PreferenceListReaderOptionsFunctor::PreferenceListReaderOptionsFunctor(const ListType& preference, const IFileReader::Options& options) : m_PreferenceList(preference), m_Options(options)
{

}

mitk::PreferenceListReaderOptionsFunctor::PreferenceListReaderOptionsFunctor(const ListType& preference, const ListType& black) : m_PreferenceList(preference), m_BlackList(black)
{

}

mitk::PreferenceListReaderOptionsFunctor::PreferenceListReaderOptionsFunctor(const ListType& preference, const ListType& black, const IFileReader::Options& options) : m_PreferenceList(preference), m_BlackList(black), m_Options(options)
{

}

bool mitk::PreferenceListReaderOptionsFunctor::operator()(IOUtil::LoadInfo &loadInfo) const
{
  auto readerItems = loadInfo.m_ReaderSelector.Get();

  auto selectedID = loadInfo.m_ReaderSelector.GetSelectedId();

  //check if the pre selected ID is on the blacklist. If true, "un"select.
  auto finding = std::find(m_BlackList.begin(), m_BlackList.end(), loadInfo.m_ReaderSelector.GetSelected().GetDescription());
  if (finding != m_BlackList.end())
  {
    selectedID = -1;
  }

  for (const auto &reader : readerItems)
  {
    finding = std::find(m_BlackList.begin(), m_BlackList.end(), reader.GetDescription());

    if (finding != m_BlackList.end())
    {
      continue;
    }

    finding = std::find(m_PreferenceList.begin(), m_PreferenceList.end(), reader.GetDescription());

    if (finding != m_PreferenceList.end())
    {
      selectedID = reader.GetServiceId();
      break;
    }

    if (selectedID==-1)
    {
      selectedID = reader.GetServiceId();
    }
  }

  if (selectedID == -1)
  {
    mitkThrow() << "No valid reader found. All available readers are black listed.";
  }

  if (!loadInfo.m_ReaderSelector.Select(selectedID))
  {
    MITK_DEBUG << "Was not able to select reader found by the PreferenceListReaderOptionsFunctor";
  }

  auto reader = loadInfo.m_ReaderSelector.GetSelected().GetReader();
  if (!m_Options.empty() && nullptr != reader)
  {
    reader->SetOptions(m_Options);
  }

  return true;
}
