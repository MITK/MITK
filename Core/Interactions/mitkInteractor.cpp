#include "mitkInteractor.h"

mitk::Interactor::Interactor(std::string type, DataTreeNode* dataTreeNode)
: mitk::StateMachine(type), m_DataTreeNode(dataTreeNode)
{
  if (m_DataTreeNode.IsNotNull())
    m_DataTreeNode->SetInteractor(this);
}

bool mitk::Interactor::ExecuteSideEffect(int, mitk::StateEvent const* , int , int )
{
/*subclasses MUST override this Method! not abstract, because of itkNewMakro can't instantiate a abstract class!*/
return true;
}

