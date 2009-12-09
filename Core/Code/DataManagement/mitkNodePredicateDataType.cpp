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

#include "mitkNodePredicateDataType.h"

#include "mitkDataTreeNode.h"
#include "mitkBaseData.h"

mitk::NodePredicateDataType::NodePredicateDataType(const char* datatype)
: NodePredicateBase()
{
  if (datatype == NULL)
    throw std::invalid_argument("NodePredicateDataType: invalid datatype");


  m_ValidDataType = datatype;
}

mitk::NodePredicateDataType::~NodePredicateDataType()
{
}


bool mitk::NodePredicateDataType::CheckNode(const mitk::DataTreeNode* node) const
{
  if (node == NULL)
    throw std::invalid_argument("NodePredicateDataType: invalid node");


  mitk::BaseData* data = node->GetData();

  if (data == NULL)
    return false;  // or should we check if m_ValidDataType == "NULL" so that nodes without data can be requested?

  return ( m_ValidDataType.compare(data->GetNameOfClass()) == 0); // return true if data type matches 
}
