#include "mitkInteractor.h"
#include <mitkDataTreeNode.h>

mitk::Interactor::Interactor(const char * type, DataTreeNode* dataTreeNode)
: mitk::StateMachine(type), m_DataTreeNode(dataTreeNode)
{
  if (m_DataTreeNode != NULL)
    m_DataTreeNode->SetInteractor(this);
}
