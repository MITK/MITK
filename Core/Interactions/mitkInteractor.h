#ifndef INTERACTOR_H_HEADER_INCLUDED
#define INTERACTOR_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkStateMachine.h"
#include <mitkDataTreeNode.h>
#include <string>

namespace mitk {

//##Documentation
//## @brief Interface for an Interactor, that is held by a DataTreeNode and holds its node
//## @ingroup Interaction
class Interactor : public StateMachine
{
public:
  mitkClassMacro(Interactor,StateMachine);

  //##Documentation
  //## @brief Constructor
  //##
  //## @params dataTreeNode is the node, this Interactor is connected to
  //## type is the type of StateMachine like declared in the XML-Configure-File
  //##
  //## Interactor connects itself to the DataTreeNode-Interactor-pointer through call of SetInteractor(this)
  Interactor(std::string type, DataTreeNode* dataTreeNode);

  //##Documentation
  //## @brief Destructor
  ~Interactor(){}

  //##Documentation
  //## @brief Executes Side-Effects
  virtual bool ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId);

protected:
  //##Documentation
  //## @brief Pointer to the data, this object handles the Interaction for
  DataTreeNode::Pointer m_DataTreeNode;
};

}//namespace mitk
#endif /* INTERACTOR_H_HEADER_INCLUDED */
