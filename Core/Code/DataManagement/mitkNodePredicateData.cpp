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

#include "mitkNodePredicateData.h"

#include "mitkDataNode.h"


mitk::NodePredicateData::NodePredicateData(mitk::BaseData* d)
: NodePredicateBase()
{
  m_DataObject = d;
}


mitk::NodePredicateData::~NodePredicateData()
{
}


bool mitk::NodePredicateData::CheckNode(const mitk::DataNode* node) const
{
  if (node == NULL)
    throw std::invalid_argument("NodePredicateData: invalid node");

  return (node->GetData() == m_DataObject);
}
