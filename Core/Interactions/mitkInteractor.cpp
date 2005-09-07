/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#include <mitkStatusBar.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <mitkOpenGLRenderer.h>
#include "mitkInteractionConst.h"
#include <vtkLinearTransform.h>
#include <itkVector.h>
#include <mitkModeOperation.h>

const std::string mitk::Interactor::XML_NODE_NAME = "interactor";

mitk::Interactor::Interactor( )
  : mitk::StateMachine( NULL ), m_DataTreeNode(NULL), m_Mode(SMDESELECTED)
{

}

mitk::Interactor::Interactor(const char * type, DataTreeNode* dataTreeNode)
  : mitk::StateMachine(type), m_DataTreeNode(dataTreeNode), m_Mode(SMDESELECTED)
{
  if (m_DataTreeNode != NULL)
    m_DataTreeNode->SetInteractor(this);
}

mitk::BaseData* mitk::Interactor::GetData() const
{
  return m_DataTreeNode->GetData();
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

bool mitk::Interactor::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent) 
{

  switch (action->GetActionId())
  {
  case AcMODEDESELECT:
    {
      this->CreateModeOperation(SMDESELECTED);
      return true;
    }
  case AcMODESELECT:
    {      
      this->CreateModeOperation(SMSELECTED);
      return true;
    }
  case AcMODESUBSELECT:
    {
      mitk::StatusBar::DisplayText("Error! in XML-Interaction: an simple Interactor can not set in sub selected", 1102);
      return false;
    }
  default:
    {
      itkWarningMacro("Message from Interactor.cpp: Action could not be understood!");
    }
  }

  return false;
}

// \todo consider time! Replace GetTimeSlicedGeometry by GetGeometry(time of stateEvent)
float mitk::Interactor::CalculateJurisdiction(StateEvent const* stateEvent) const
{
  float returnvalue = 0.0;
  //if it is a key event that can be handled in the current state, then return 0.5
  mitk::DisplayPositionEvent const  *disPosEvent = dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());

  //Key event handling:
  if (disPosEvent == NULL)
  {
    //check, if the current state has a transition waiting for that key event.
    if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
    {
      return 0.5;
    }
    else
    {
      return 0;
    }
  }

  //Mouse event handling:
  //on MouseMove do nothing! reimplement if needed differently
  if (stateEvent->GetEvent()->GetType() == mitk::Type_MouseMove)
  {
    return 0;
  }

  //compute the center of the data taken care of
  const mitk::BoundingBox *bBox = GetData()->GetTimeSlicedGeometry()->GetBoundingBox();
  if (bBox == NULL)
    return 0;

  mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
  if (posEvent == NULL) //2D information from a 3D window
  {
    //get camera and calculate the distance between the center of this boundingbox and the camera
    mitk::OpenGLRenderer* oglRenderer = dynamic_cast<mitk::OpenGLRenderer*>(stateEvent->GetEvent()->GetSender());
    if (oglRenderer == NULL)
      return 0;

    vtkCamera* camera = oglRenderer->GetVtkRenderer()->GetActiveCamera();
    if (camera == NULL)
    {
      assert(disPosEvent->GetSender()!=NULL);
      return 0;
    }
#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
    double normal[3];
#else
    float normal[3];
#endif
    camera->GetViewPlaneNormal(normal);

    mitk::BoundingBox::PointType center,n;
    vtk2itk(normal, n);
    returnvalue = center.SquaredEuclideanDistanceTo( n );
    //map between 0.5 and 1
  }
  else//3D information from a 2D window.
  {
    //transforming the Worldposition to local coordinatesystem
    mitk::Point3D point;
    GetData()->GetTimeSlicedGeometry()->WorldToIndex(posEvent->GetWorldPosition(), point);

    //distance between center and point 
    mitk::BoundingBox::PointType center = bBox->GetCenter();
    returnvalue = point.EuclideanDistanceTo(center);

    //now compared to size of boundingbox to get between 0 and 1;
    returnvalue = returnvalue/( (bBox->GetMaximum().EuclideanDistanceTo(bBox->GetMinimum() ) ) );

    //safety: if by now returnvalue is not in 0 and 1, then return 1!
    if (returnvalue>1 ||returnvalue<0)
      return 0;

    //shall be 1 if short length to center
    returnvalue = 1 - returnvalue;

    //check if the given position lies inside the data-object
    if (bBox->IsInside(point))
    {
      //mapped between 0,5 and 1
      returnvalue = 0.5 + (returnvalue / 2);
    }
    else
    {
      //set it in range between 0 and 0.5
      returnvalue = returnvalue / 2;
    }
  }
  return returnvalue;
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

  //##
const std::string& mitk::Interactor::GetXMLNodeName() const
{
  return XML_NODE_NAME;
}

void mitk::Interactor::SetDataTreeNode( DataTreeNode* dataTreeNode )
{
    m_DataTreeNode = dataTreeNode;
}
