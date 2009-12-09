/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 14:46:19 +0100 (Di, 11 Dez 2007) $
Version:   $Revision: 13129 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNodePredicateFirstLevel.h"

mitk::NodePredicateFirstLevel::NodePredicateFirstLevel(mitk::DataStorage* ds)
: NodePredicateBase(), m_DataStorage(ds)
{
}

mitk::NodePredicateFirstLevel::~NodePredicateFirstLevel()
{
}


bool mitk::NodePredicateFirstLevel::CheckNode(const mitk::DataTreeNode* node) const
{
  if (node == NULL)
    throw std::invalid_argument("NodePredicateFirstLevel: invalid node");


  if(m_DataStorage.IsNull())
    throw std::invalid_argument("NodePredicateFirstLevel: DataStorage is invalid");

  mitk::DataStorage::SetOfObjects::ConstPointer list = m_DataStorage->GetSources(node, NULL, true);
  return (list->Size() == 0);
}
