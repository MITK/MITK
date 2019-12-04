/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNodePredicateSource.h"

mitk::NodePredicateSource::NodePredicateSource(mitk::DataNode *n, bool allsources, mitk::DataStorage *ds)
  : NodePredicateBase(), m_BaseNode(n), m_SearchAllSources(allsources), m_DataStorage(ds)
{
}

mitk::NodePredicateSource::~NodePredicateSource()
{
}

bool mitk::NodePredicateSource::CheckNode(const mitk::DataNode *childNode) const
{
  if (m_DataStorage && m_BaseNode)
  {
    const mitk::DataStorage::SetOfObjects::STLContainerType sources =
      m_DataStorage.Lock()->GetSources(childNode, nullptr, !m_SearchAllSources)->CastToSTLConstContainer();

    return std::find(sources.cbegin(), sources.cend(), m_BaseNode.Lock()) != sources.cend();
  }

  return false;
}
