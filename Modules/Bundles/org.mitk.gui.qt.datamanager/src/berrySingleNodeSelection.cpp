#include "berrySingleNodeSelection.h"

#include "mitkDataTreeNode.h"

namespace berry
{

  void SingleNodeSelection::SetNode( mitk::DataTreeNode* _SelectedNode )
  {
    m_Node = _SelectedNode;
  }

  mitk::DataTreeNode* SingleNodeSelection::GetNode() const
  {
    return m_Node;
  }

  bool SingleNodeSelection::IsEmpty() const
  {
    return ( m_Node == 0 );
  }

}
