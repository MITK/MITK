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

#include <mitkDataTreeFilterFunctions.h>
#include "mitkDataNode.h"

namespace mitk
{

bool DataTreeFilterFunction::operator()(DataNode* node) const
{
  return NodeMatches(node);
}

bool IsDataNode::NodeMatches(DataNode* node) const
{
  return ( node != NULL );
}

DataTreeFilterFunction* IsDataNode::Clone() const
{
  return new IsDataNode();
}

bool IsGoodDataNode::NodeMatches(DataNode* node) const
{
  return ( node != NULL && node->GetData() );
}

DataTreeFilterFunction* IsGoodDataNode::Clone() const
{
  return new IsGoodDataNode();
}

bool IsInResultSet::NodeMatches(DataNode* node) const
{
  if ((node == NULL) || (m_ResultSet.empty()))
    return false;

  return (std::find(m_ResultSet.begin(), m_ResultSet.end(), node) != m_ResultSet.end()); // search for node in resultset
}

DataTreeFilterFunction* IsInResultSet::Clone() const
{
  return new IsInResultSet(m_ResultSet);
}

} // namespace

