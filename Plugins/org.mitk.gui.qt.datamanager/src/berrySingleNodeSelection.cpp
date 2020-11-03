/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    return ( m_Node == nullptr );
  }

}
