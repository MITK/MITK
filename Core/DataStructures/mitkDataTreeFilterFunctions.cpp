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
#include "mitkDataTreeNode.h"

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

bool IsInResultSet::NodeMatches(DataTreeNode* node) const
{
  if ((node == NULL) || (m_ResultSet.IsNull()))
    return false;

  DataStorage::SetOfObjects::STLContainerType rs = m_ResultSet->CastToSTLConstContainer();
  return (std::find(rs.begin(), rs.end(), mitk::DataTreeNode::Pointer(node)) != rs.end()); // search for node in resultset
}

DataTreeFilterFunction* IsInResultSet::Clone() const
{
  return new IsInResultSet(m_ResultSet);
}

} // namespace

