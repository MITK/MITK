#ifndef INTERACTOR_H_HEADER_INCLUDED
#define INTERACTOR_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkStateMachine.h"
#include <mitkOperationActor.h>

#include <string>

namespace mitk {

class DataTreeNode;

//##Documentation
//## @brief Interface for an Interactor.
//##
//## The Interactor is held with a SmartPointer by a DataTreeNode 
//## and holds its Node with a Pointer. That way Smartpointer doesn't build a circle.
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
  virtual bool ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId) = 0;

protected:
  //##Documentation
  //## @brief Pointer to the data, this object handles the Interaction for
  DataTreeNode* m_DataTreeNode;

  //##Documentation
  //## @brief pointer to the Data
  OperationActor* m_Data;


};

}//namespace mitk
#endif /* INTERACTOR_H_HEADER_INCLUDED */
