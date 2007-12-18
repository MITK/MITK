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
#include <mitkDataTreeNode.h>
#include <mitkDisplayPositionEvent.h>
#include <mitkPositionEvent.h>
#include <mitkGeometry3D.h>
#include <mitkAction.h>
#include <mitkOperationEvent.h>
#include <mitkStateEvent.h>
#include <mitkState.h>
#include <mitkUndoController.h>
#include <mitkStatusBar.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include "mitkInteractionConst.h"
#include <vtkLinearTransform.h>
#include <itkVector.h>
#include <mitkModeOperation.h>
#include "mitkGlobalInteraction.h"

const std::string mitk::Interactor::XML_NODE_NAME = "interactor";

/**
* @brief Obsolete! Only to maintain interface!
**/
//mitk::Interactor::Interactor( )
//  : mitk::StateMachine( NULL ), m_DataTreeNode(NULL), m_Mode(SMDESELECTED)
//{
//}

mitk::Interactor::Interactor(const char * type, DataTreeNode* dataTreeNode)
  : mitk::StateMachine(type), m_DataTreeNode(dataTreeNode), m_Mode(SMDESELECTED)
{
  if (m_DataTreeNode != NULL)
    m_DataTreeNode->SetInteractor(this);
}

mitk::BaseData* mitk::Interactor::GetData() const
{
  if (m_DataTreeNode != NULL)
    return m_DataTreeNode->GetData();
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
  mitk::ModeOperation* doOp = new mitk::ModeOperation(OpMODECHANGE, mode);
  if (m_UndoEnabled)
  {
    mitk::ModeOperation* undoOp = new mitk::ModeOperation(OpMODECHANGE, this->GetMode());
    OperationEvent *operationEvent = new OperationEvent(this, doOp, undoOp);
    m_UndoController->SetOperationEvent(operationEvent);
  }
  this->ExecuteOperation(doOp);
}

bool mitk::Interactor::ExecuteAction(Action* action, mitk::StateEvent const* /*stateEvent*/) 
{
  GlobalInteraction* global = mitk::GlobalInteraction::GetInstance();
  if (global == NULL)
    itkWarningMacro("Message from Interactor.cpp: GlobalInteraction == NULL! Check use of Interactor!");
  
  switch (action->GetActionId())
  {
  case AcMODEDESELECT:
    {
      if( this->GetMode() !=  SMDESELECTED)
      {
        this->CreateModeOperation(SMDESELECTED);
        global->RemoveFromSelectedInteractors(this);
      }
      return true;
    }
  case AcMODESELECT:
    { 
      if( this->GetMode() !=  SMSELECTED)
      {
        this->CreateModeOperation(SMSELECTED);
        global->AddToSelectedInteractors(this);
      }
      return true;
    }
  case AcMODESUBSELECT:
    {
      mitk::StatusBar::GetInstance()->DisplayText("Error! in XML-Interaction: an simple Interactor can not set in sub selected", 1102);
      return false;
    }
  default:
    {
      itkWarningMacro("Message from Interactor.cpp: Action could not be understood!");
    }
  }

  return false;
}

float mitk::Interactor::CalculateJurisdiction(StateEvent const* stateEvent) const
{
    //retunrValue for boundingbox
  float returnvalueBB = 0.0, 
    //retunrvalue for a existing transition
    returnvalueTransition = 0.0, 
    //returnvalue for an existing key transition
    returnvalueKey = 0.0;

  //if it is a key event that can be handled in the current state
  mitk::DisplayPositionEvent const  *disPosEvent = dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());

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
  if (stateEvent->GetEvent()->GetType() == mitk::Type_MouseMove)
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
    const mitk::BoundingBox *bBox = GetData()->GetUpdatedTimeSlicedGeometry()->GetBoundingBox();
    if (bBox == NULL)
      return 0;


    mitk::DisplayPositionEvent const  *event = dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());
    if (event != NULL)
    {
      //transforming the Worldposition to local coordinatesystem
      mitk::Point3D point;
      GetData()->GetTimeSlicedGeometry()->WorldToIndex(event->GetWorldPosition(), point);

      //distance between center and point 
      mitk::BoundingBox::PointType center = bBox->GetCenter();
      returnvalueBB = point.EuclideanDistanceTo(center);

      //now compared to size of boundingbox to get between 0 and 1;
      returnvalueBB = returnvalueBB/( (bBox->GetMaximum().EuclideanDistanceTo(bBox->GetMinimum() ) ) );

      //safety: if by now returnvalue is not in 0 and 1, then return 1!
      if (returnvalueBB>1 ||returnvalueBB<0)
        returnvalueBB = 0;

      //shall be 1 if short length to center
      returnvalueBB = 1 - returnvalueBB;

      //check if the given position lies inside the data-object
      if (bBox->IsInside(point))
      {
        //mapped between 0,5 and 1
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

void mitk::Interactor::SetDataTreeNode( DataTreeNode* dataTreeNode )
{
    m_DataTreeNode = dataTreeNode;
}
