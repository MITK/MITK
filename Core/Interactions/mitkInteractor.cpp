#include "mitkInteractor.h"

mitk::Interactor::Interactor(std::string type, DataTreeNode* dataTreeNode)
: mitk::StateMachine(type), m_DataTreeNode(dataTreeNode)
{
  if (m_DataTreeNode.IsNotNull())
    m_DataTreeNode->SetInteractor(this);
}
