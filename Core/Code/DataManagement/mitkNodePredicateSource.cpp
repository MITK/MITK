/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
