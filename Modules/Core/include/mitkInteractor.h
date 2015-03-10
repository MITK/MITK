/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef INTERACTOR_H_HEADER_INCLUDED
#define INTERACTOR_H_HEADER_INCLUDED

#include <MitkCoreExports.h>
#include "mitkStateMachine.h"
#include "mitkGeometry3D.h"

#include <string>

namespace mitk {

class DataNode;
class BaseData;

//##Documentation
//## @brief Interface for an Interactor.
//##
//## The Interactor is held with a SmartPointer by a DataNode
//## and holds its Node with a Pointer. That way Smartpointer doesn't build a circle.
//## Different Modes: In order to not send Events to all StateMachines, a StateMachine can be
//## in three different modes:
//## DESELECTED: this statemachine doesn't wait for an event
//## SELECTED: this statemachine just has handled an event and waits for the next one
//## SUBSELECTED: depricate; was used for hierarchical statemachines before.
//## Guidelines for the modevalues: Selected if the coresponding data is selected, deselected if deselect of data.
//##
//## In moving the machine is selected. After a new insert the machine is selected, since the data is also selected
//## In method ExecuteAction(..) the different actions are divided up through switch/case statements. Each block has to check
//## the appropriate type of event to process the actions. Especially in guarding states (a state, that checks certain conditions (e.g. is picked)
//## the according Event must be called to continue in states. No return false here!
//## @ingroup Interaction
/**
 * \deprecatedSince{2013_03} mitk::Interactor is deprecated. Use mitk::DataInteractor instead.
 * Refer to \see DataInteractionPage  for general information about the concept of the new implementation.
 */
class MITKCORE_EXPORT Interactor : public StateMachine
{
public:
    mitkClassMacro(Interactor, StateMachine);

    /**
    * @brief NewMacro with two parameters for calling itk::Lightobject::New(..) method
    **/
    mitkNewMacro2Param(Self, const char*, DataNode*);

  //##Documentation
  //##@brief Enumeration of the different modes an Interactor can be into.
  //## See class documentation for further details
  enum SMMode
  {
    SMDESELECTED = 0,
    SMSELECTED,
    SMSUBSELECTED
  };

  typedef SMMode ModeType;

  //##Documentation
  //## @brief Get the Mode of the Interactor. Use enum SMMode for return parameter
  SMMode GetMode() const;

  //##Documentation
  //## @brief Check the interaction mode
  bool IsNotSelected() const;

  //##Documentation
  //## @brief Check the interaction mode
  bool IsSelected() const;

  //##Documentation
  //## @brief calculates how good the data, this statemachine handles, is hit by the event.
  //##
  //## Returns a value between 0 and 1
  //## where 0 represents not responsible and 1 represents definitive responsible!
  //## Standard function to override if needed.
  //## (Used by GlobalInteraction to decide which DESELECTED statemachine to send the event to.)
  virtual float CanHandleEvent(StateEvent const* stateEvent) const;

  /**
  * @brief Updates the current TimeStep according to the associated data and calls Superclass::HandleEvent()
  **/
  bool HandleEvent(StateEvent const* stateEvent);

  /**
  * @brief Method to call if the associated data has changed by the user (loading of data)
  * This method is called by DataNode::SetData() to tell the interactor to reinitialize.
  * This method should be overwritten by specialized interactors.
  * (e.g. PointSetInteractor: go to the right state according to number of loaded points)
  * Note: It will not be called when the data gets modified (e.g. adding / removing points to a PointSet)
  **/
  virtual void DataChanged(){};

  //##Documentation
  //## @brief adds handling of operations used for mode change. Unrecognized Operations are send to Superclass.
  //## *ATTENTION*: THIS METHOD SHOULD NOT BE CALLED FROM OTHER CLASSES DIRECTLY!
  virtual void ExecuteOperation(Operation* operation);

  static const std::string XML_NODE_NAME;

protected:
  /**
  * @brief Constructor
  * @param dataNode is the node, this Interactor is connected to
  * @param type is the type of StateMachine like declared in the XML-Configure-File
  *
  * Interactor connects itself to the DataNode-Interactor-pointer through call of SetInteractor(this)
  **/
  Interactor(const char * type, DataNode* dataNode);

  /**
  * @brief Destructor
  **/
  ~Interactor(){}

  bool OnModeSelect(Action* action, StateEvent const*);
  bool OnModeDeselect(Action* action, StateEvent const*);
  bool OnModeSubSelect(Action* action, StateEvent const*);

  //##Documentation
  virtual const std::string& GetXMLNodeName() const;

  //##Documentation
  //## @brief creates a ModeOperation with the transmitted mode and sends it to this. Undo supported!
  void CreateModeOperation(ModeType mode);

  //##Documentation
  //## @brief convenience method for accessing the data contained in the
  //## node to which this interactor is associated to
  BaseData* GetData() const;


  //##Documentation
  //## @brief Used by friend class DataNode
  virtual void SetDataNode( DataNode* dataNode );

  /**
  * @brief Derived from superclass to also check if enough timesteps are instantiated in m_CurrentStateVector
  * The number of timesteps is read from the dedicated data.
  * @param[in] timeStep The timeStep that the statemachine has to be set to
  **/
  virtual void UpdateTimeStep(unsigned int timeStep);

  //##Documentation
  //## @brief Pointer to the data, this object handles the Interaction for
  DataNode* m_DataNode;

  //##Documentation
  //## @brief Mode of Selection
  ModeType m_Mode;

  friend class DataNode;
};

}//namespace mitk
#endif /* INTERACTOR_H_HEADER_INCLUDED */


