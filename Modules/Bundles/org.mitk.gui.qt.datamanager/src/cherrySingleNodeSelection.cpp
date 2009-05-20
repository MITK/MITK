#include "cherrySingleNodeSelection.h"

#include "mitkDataTreeNode.h"

namespace cherry
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
