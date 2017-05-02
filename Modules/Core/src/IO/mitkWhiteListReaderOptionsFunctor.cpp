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

#include "mitkWhiteListReaderOptionsFunctor.h"

mitk::WhiteListReaderOptionsFunctor::WhiteListReaderOptionsFunctor()
{

}

mitk::WhiteListReaderOptionsFunctor::WhiteListReaderOptionsFunctor(const ListType& white, const ListType& black) : m_WhiteList(white), m_BlackList(black)
{

}

bool mitk::WhiteListReaderOptionsFunctor::operator()(IOUtil::LoadInfo &loadInfo) const
{
  auto readerItems = loadInfo.m_ReaderSelector.Get();

  auto selectedID = loadInfo.m_ReaderSelector.GetSelectedId();

  //check if the pre selected ID is on the blacklist. If true, "un"select.
  auto finding = std::find(m_BlackList.begin(), m_BlackList.end(), loadInfo.m_ReaderSelector.GetSelected().GetDescription());
  if (finding != m_BlackList.end())
  {
    selectedID = -1;
  }

  for (auto reader : readerItems)
  {
    finding = std::find(m_BlackList.begin(), m_BlackList.end(), reader.GetDescription());

    if (finding != m_BlackList.end())
    {
      continue;
    }

    finding = std::find(m_WhiteList.begin(), m_WhiteList.end(), reader.GetDescription());

    if (finding != m_WhiteList.end())
    {
      selectedID = reader.GetServiceId();
      break;
    }

    if (selectedID==-1)
    {
      selectedID = reader.GetServiceId();
    }
  }

  loadInfo.m_ReaderSelector.Select(selectedID);

  return true;
}
