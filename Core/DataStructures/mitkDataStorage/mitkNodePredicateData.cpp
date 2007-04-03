/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-02-22 17:32:15 +0100 (Thu, 22 Feb 2007) $
Version:   $Revision: 9465 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNodePredicateData.h"

#include "mitkDataTreeNode.h"


mitk::NodePredicateData::NodePredicateData(mitk::BaseData* d)
: NodePredicateBase()
{
  m_DataObject = d;
}


mitk::NodePredicateData::~NodePredicateData()
{
}


bool mitk::NodePredicateData::CheckNode(const mitk::DataTreeNode* node) const
{
  if (node == NULL)
    throw 1;  // Insert Exception Handling here

  return (node->GetData() == m_DataObject);
}
