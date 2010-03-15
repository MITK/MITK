/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkInteractor.h"
#include <mitkDataNode.h>
#include <mitkDisplayPositionEvent.h>
#include <mitkPositionEvent.h>
#include <mitkGeometry3D.h>
#include <mitkAction.h>
#include <mitkOperationEvent.h>
#include <mitkStateEvent.h>
#include <mitkState.h>
#include <mitkUndoController.h>
//#include <mitkStatusBar.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include "mitkInteractionConst.h"
#include <vtkLinearTransform.h>
#include <itkVector.h>
#include <mitkModeOperation.h>
#include "mitkGlobalInteraction.h"

const std::string mitk::Interactor::XML_NODE_NAME = "interactor";

mitk::Interactor::Interactor(const char * type, DataNode* dataTreeNode)
: StateMachine(type), 
  m_DataNode(dataTreeNode), 
  m_Mode(SMDESELECTED)
{
  if (m_DataNode != NULL)
    m_DataNode->SetInteractor(this);

  // handle these actions in those Methods
  CONNECT_ACTION( AcMODEDESELECT,  OnModeDeselect );
  CONNECT_ACTION( AcMODESELECT,    OnModeSelect );
  CONNECT_ACTION( AcMODESUBSELECT, OnModeSubSelect );
}

mitk::BaseData* mitk::Interactor::GetData() const
{
  if (m_DataNode != NULL)
    return m_DataNode->GetData();
  else 
    return NULL;
}

mitk::Interactor::SMMode mitk::Interactor::GetMode() const
{
  return m_Mode;
}

bool mitk::Interactor::IsNotSelected() const 
{
  return (m_Mode==SMDESELECTED);
}

bool mitk::Interactor::IsSelected() const 
{
  return (m_Mode!=SMDESELECTED);
}

void mitk::Interactor::CreateModeOperation(ModeType mode)
{
  ModeOperation* doOp = new ModeOperation(OpMODECHANGE, mode);
  if (m_UndoEnabled)
  {
    ModeOperation* undoOp = new ModeOperation(OpMODECHANGE, this->GetMode());
    OperationEvent *operationEvent = new OperationEvent(this, doOp, undoOp);
    m_UndoController->SetOperationEvent(operationEvent);
  }
  this->ExecuteOperation(doOp);
}

bool mitk::Interactor::OnModeDeselect(Action* /*action*/, StateEvent const*)
{
  GlobalInteraction* global = GlobalInteraction::GetInstance();
  if (global == NULL)
    itkWarningMacro("Message from Interactor.cpp: GlobalInteraction == NULL! Check use of Interactor!");

  if( this->GetMode() !=  SMDESELECTED)
  {
    this->CreateModeOperation(SMDESELECTED);
    global->RemoveFromSelectedInteractors(this);
  }
  return true;
}

bool mitk::Interactor::OnModeSelect(Action* /*action*/, StateEvent const*)
{
  GlobalInteraction* global = GlobalInteraction::GetInstance();
  if (global == NULL)
    itkWarningMacro("Message from Interactor.cpp: GlobalInteraction == NULL! Check use of Interactor!");

  if( this->GetMode() !=  SMSELECTED)
  {
    this->CreateModeOperation(SMSELECTED);
    global->AddToSelectedInteractors(this);
  }
  return true;
}

bool mitk::Interactor::OnModeSubSelect(Action* /*action*/, StateEvent const*)
{
  //StatusBar::GetInstance()->DisplayText("Error! in XML-Interaction: an simple Interactor can not set in sub selected", 1102);
  return false;
}

float mitk::Interactor::CalculateJurisdiction(StateEvent const* stateEvent) const
{
    //return value for boundingbox
  float returnvalueBB = 0.0, 
    //return value for a existing transition
    returnvalueTransition = 0.0, 
    //return value for an existing key transition
    returnvalueKey = 0.0;

  //if it is a key event that can be handled in the current state
  DisplayPositionEvent const  *disPosEvent = dynamic_cast <const DisplayPositionEvent *> (stateEvent->GetEvent());

  //Key event handling:
  if (disPosEvent == NULL)
  {
    //check, if the current state has a transition waiting for that key event.
    if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
    {
      returnvalueKey = 0.5;
    }
  }

  //Mouse event handling:
  //on MouseMove do nothing! reimplement if needed differently
  if (stateEvent->GetEvent()->GetType() == Type_MouseMove)
  {
    return 0;
  }

  //if the event can be understood and if there is a transition waiting for that event
  if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
  {
    returnvalueTransition = 0.5;//it can be understood
  }
  
  //compute the center of the data taken care of if != NULL
  if (GetData() != NULL)
  {
    const BoundingBox *bBox = GetData()->GetUpdatedTimeSlicedGeometry()->GetBoundingBox();
    if (bBox == NULL)
      return 0;


    DisplayPositionEvent const  *event = dynamic_cast <const DisplayPositionEvent *> (stateEvent->GetEvent());
    if (event != NULL)
    {
      //transforming the world position to local coordinate system
      Point3D point;
      GetData()->GetTimeSlicedGeometry()->WorldToIndex(event->GetWorldPosition(), point);

      //distance between center and point 
      BoundingBox::PointType center = bBox->GetCenter();
      returnvalueBB = point.EuclideanDistanceTo(center);

      // now check if object bounding box has a non-zero size
      float bBoxSize = bBox->GetMaximum().EuclideanDistanceTo(bBox->GetMinimum() );
      if( bBoxSize < 0.00001 ) return 0;      // bounding box too small?

      //now compared to size of bounding box to get value between 0 and 1;
      returnvalueBB = returnvalueBB/bBoxSize;

      //safety: if by now return value is not in [0,1], then return 0!
      if (returnvalueBB>1 || returnvalueBB<0)
        returnvalueBB = 0;

      // A return value of 1 is good, 0 is bad -> reverse value
      returnvalueBB = 1 - returnvalueBB;

      //check if the given position lies inside the data object
      if (bBox->IsInside(point))
      {
        //mapped between 0.5 and 1
        returnvalueBB = 0.5 + (returnvalueBB/ 2);
      }
      else
      {
        //set it in range between 0 and 0.5
        returnvalueBB = returnvalueBB / 2;
      }
    }
  }
  //else
  //  itkWarningMacro("Data of Interactor is NULL! Please check setup of Interactors!");

  return std::max(returnvalueBB, std::max(returnvalueKey, returnvalueTransition));
}

void mitk::Interactor::ExecuteOperation(Operation* operation)
{
  switch (operation->GetOperationType())
  {
  case OpMODECHANGE:
    {
      ModeOperation *modeOp = dynamic_cast<ModeOperation*>(operation);
      if (modeOp)
      {
        m_Mode = modeOp->GetMode();
      }
    }
    break;
  default:
    Superclass::ExecuteOperation(operation);
  }
}

const std::string& mitk::Interactor::GetXMLNodeName() const
{
  return XML_NODE_NAME;
}

void mitk::Interactor::SetDataNode( DataNode* dataTreeNode )
{
  m_DataNode = dataTreeNode;
  
  //check for the number of time steps and initialize the vector of CurrentStatePointer accordingly
  if (m_DataNode != NULL)
  {
    mitk::BaseData* data = dataTreeNode->GetData();
    if (data != NULL)
    {
      unsigned int timeSteps = data->GetTimeSteps();
      //expand the list of StartStates according to the number of timesteps in data
      if (timeSteps > 1)
        this->InitializeStartStates(timeSteps);
    }
  }
}

void mitk::Interactor::UpdateTimeStep(unsigned int timeStep)
{
  //check if the vector of StartStates contains enough pointers to use timeStep
  if (timeStep >= 1)
  {
    // Make sure that the data (if time-resolved) has enough entries;
    // if not, create the required extra ones (empty)
    if (m_DataNode!= NULL)
      if (m_DataNode->GetData()!= NULL)
        m_DataNode->GetData()->Expand(timeStep+1); //+1 becuase the vector starts with 0 and the timesteps with 1
    
    //now check for this object
    this->ExpandStartStateVector(timeStep+1); //nothing is changed if the number of timesteps in data equals the number of startstates held in statemachine
  }

  //set the time to the given time
  Superclass::UpdateTimeStep(timeStep);

  //time has to be up-to-date
  //check and throw an exception if not so
  if (timeStep != m_TimeStep)
    itkExceptionMacro(<<"Time is invalid. Take care of synchonization!");
}

bool mitk::Interactor::HandleEvent(StateEvent const* stateEvent)
{
  //update the Time and then call Superclass
  if (stateEvent != NULL)
  {
    mitk::Event const* event = stateEvent->GetEvent();
    if (event != NULL)
    {
      mitk::BaseRenderer* sender = event->GetSender();
      if (sender != NULL)
      {
        //Get the TimeStep according to CurrentWorldGeometry2D
        unsigned int currentTimeStep = sender->GetTimeStep();
        if (currentTimeStep != m_TimeStep)
          this->UpdateTimeStep(currentTimeStep);
      }
    }
  }
  return Superclass::HandleEvent(stateEvent);
}
