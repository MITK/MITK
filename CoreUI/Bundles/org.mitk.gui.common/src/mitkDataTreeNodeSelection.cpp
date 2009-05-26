/*=========================================================================

 Program:   openCherry Platform
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

#include "mitkDataTreeNodeSelection.h"

namespace mitk
{

DataTreeNodeSelection::DataTreeNodeSelection()
{

}

DataTreeNodeSelection::DataTreeNodeSelection(const NodeContainer& nodes)
: m_Nodes(nodes)
{

}

const std::vector<DataTreeNode::Pointer>&
DataTreeNodeSelection::GetDataTreeNodes() const
{
  return m_Nodes;
}

unsigned int
DataTreeNodeSelection::GetSize() const
{
  return m_Nodes.size();
}

bool
DataTreeNodeSelection::IsEmpty() const
{
  return m_Nodes.empty();
}

}
