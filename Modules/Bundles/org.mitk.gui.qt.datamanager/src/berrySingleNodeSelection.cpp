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
