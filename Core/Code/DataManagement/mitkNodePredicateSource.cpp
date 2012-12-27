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

#include "mitkNodePredicateSource.h"

mitk::NodePredicateSource::NodePredicateSource(mitk::DataNode* n, bool allsources, mitk::DataStorage* ds)
: NodePredicateBase(), m_BaseNode(n), m_SearchAllSources(allsources), m_DataStorage(ds)
{
}

mitk::NodePredicateSource::~NodePredicateSource()
{
}


bool mitk::NodePredicateSource::CheckNode(const mitk::DataNode* childNode) const
{
  if(m_DataStorage && m_BaseNode)
  {
    const mitk::DataStorage::SetOfObjects::STLContainerType sources =
      m_DataStorage->GetSources(childNode, 0, !m_SearchAllSources)->CastToSTLConstContainer();

    return std::find(sources.begin(), sources.end(), m_BaseNode) != sources.end();
  }

  return false;
}
