#include "mitkInteractor.h"
#include <mitkDataTreeNode.h>

mitk::Interactor::Interactor(std::string type, DataTreeNode* dataTreeNode)
: mitk::StateMachine(type), m_DataTreeNode(dataTreeNode)
{
  if (m_DataTreeNode != NULL)
    m_DataTreeNode->SetInteractor(this);
}
