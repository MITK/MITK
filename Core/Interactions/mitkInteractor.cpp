#include "mitkInteractor.h"
#include <mitkDataTreeNode.h>

mitk::Interactor::Interactor(const char * type, DataTreeNode* dataTreeNode)
: mitk::StateMachine(type), m_DataTreeNode(dataTreeNode), m_Mode(SMSELECTED)
{
  if (m_DataTreeNode != NULL)
    m_DataTreeNode->SetInteractor(this);
}

void mitk::Interactor::SetMode(int mode)
{
  if ((-1<mode)&&(mode<3))//if mode in range from 0 to 2
    m_Mode = mode;
}

const int mitk::Interactor::GetMode() const
{
  return m_Mode;
}

