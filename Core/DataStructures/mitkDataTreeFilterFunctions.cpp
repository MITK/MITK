/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include <mitkDataTreeFilterFunctions.h>

namespace mitk
{

bool DataTreeFilterFunction::operator()(DataTreeNode* node) const
{
  return NodeMatches(node);
}

bool IsDataTreeNode::NodeMatches(DataTreeNode* node) const
{
  return ( node != NULL );
}

DataTreeFilterFunction* IsDataTreeNode::Clone() const
{
  return new IsDataTreeNode();
}

bool IsGoodDataTreeNode::NodeMatches(DataTreeNode* node) const
{
  return ( node != NULL && node->GetData() );
}

DataTreeFilterFunction* IsGoodDataTreeNode::Clone() const
{
  return new IsGoodDataTreeNode();
}

} // namespace

