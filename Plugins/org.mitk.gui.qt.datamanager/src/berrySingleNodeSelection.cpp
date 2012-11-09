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

#include "berrySingleNodeSelection.h"

#include "mitkDataNode.h"

namespace berry
{

  void SingleNodeSelection::SetNode( mitk::DataNode* _SelectedNode )
  {
    m_Node = _SelectedNode;
  }

  mitk::DataNode* SingleNodeSelection::GetNode() const
  {
    return m_Node;
  }

  bool SingleNodeSelection::IsEmpty() const
  {
    return ( m_Node == 0 );
  }

}
