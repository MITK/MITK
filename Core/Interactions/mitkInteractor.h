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
//## Different Modes: In order to work with hierarchical StateMachines and not to send Events to all StateMachines, a StateMachine can be 
//## in three different modes: 
//## DESELECTED: this statemachine doesn't wait for an event
//## SELECTED: this statemachine just has handled an event and waits for the next one
//## SUBSELECTED: a hierarchical lower statemachine has just handled an event so this statemachine wants to pass the event to this machine
//## Guidelines for the modevalues: Selected if the coresponding data is selected, deselected if deselect of data, subselect if submachine is selected
//## in moving the machine is selected. After a new insert the machine is selected, since the data is also selected
//## @ingroup Interaction
class Interactor : public StateMachine
{
public:

  //##Documentation
  //##@brief Enumeration of the different modes an Interactor can be into. 
  //## See class documentation for further details
  typedef enum SMMode
  {
    SMDESELECTED = 0,
    SMSELECTED,
    SMSUBSELECTED
  };

  mitkClassMacro(Interactor,StateMachine);

  //##Documentation
  //## @brief Constructor
  //##
  //## @params dataTreeNode is the node, this Interactor is connected to
  //## type is the type of StateMachine like declared in the XML-Configure-File
  //##
  //## Interactor connects itself to the DataTreeNode-Interactor-pointer through call of SetInteractor(this)
  Interactor(const char * type, DataTreeNode* dataTreeNode);

  //##Documentation
  //## @brief Destructor
  ~Interactor(){}

  //##Documentation
  //## @brief Set the Mode of the Interactor to Selected, Subselected or Deselected
  //## use enum SMMode for parameter
  void SetMode(int mode);

  //##Documentation
  //## @brief Get the Mode of the Interactor. Use enum SMMode for return parameter
  const int GetMode() const;

  //##Documentation
  //## @brief Executes Side-Effects
  virtual bool ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId) = 0;

protected:
  //##Documentation
  //## @brief Pointer to the data, this object handles the Interaction for
  DataTreeNode* m_DataTreeNode;

  //##Documentation
  //## @brief Mode of Selection
  int m_Mode;

};

}//namespace mitk
#endif /* INTERACTOR_H_HEADER_INCLUDED */
