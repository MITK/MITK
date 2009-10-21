/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-10-02 16:21:08 +0200 (Do, 02 Okt 2008) $
Version:   $Revision: 13129 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPlanarFigureInteractor.h"
#include "mitkPointOperation.h"
#include "mitkPositionEvent.h"
#include "mitkPlanarFigure.h"
#include "mitkStatusBar.h"
#include "mitkDataTreeNode.h"
#include "mitkInteractionConst.h"
#include "mitkAction.h"
#include "mitkStateEvent.h"
#include "mitkOperationEvent.h"
#include "mitkUndoController.h"
#include "mitkStateMachineFactory.h"
#include "mitkStateTransitionOperation.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"


//how precise must the user pick the point
//default value
mitk::PlanarFigureInteractor
::PlanarFigureInteractor(const char * type, DataTreeNode* dataTreeNode, int /* n */ )
: Interactor( type, dataTreeNode ),
  m_Precision( 6.5 )
{
}

mitk::PlanarFigureInteractor::~PlanarFigureInteractor()
{
}

void mitk::PlanarFigureInteractor::SetPrecision( mitk::ScalarType precision )
{
  m_Precision = precision;
}

// Overwritten since this class can handle it better!
float mitk::PlanarFigureInteractor
::CalculateJurisdiction(StateEvent const* stateEvent) const
{
  float returnValue = 0.0;
  

  // If it is a key event that can be handled in the current state,
  // then return 0.5
  mitk::DisplayPositionEvent const *disPosEvent =
    dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());

  // Key event handling:
  if (disPosEvent == NULL)
  {
    // Check if the current state has a transition waiting for that key event.
    if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
    {
      return 0.5;
    }
    else
    {
      return 0.0;
    }
  }

  //on MouseMove do nothing!
  //if (stateEvent->GetEvent()->GetType() == mitk::Type_MouseMove)
  //{
  //  return 0.0;
  //}

  //if the event can be understood and if there is a transition waiting for that event
  if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
  {
    returnValue = 0.5;//it can be understood
  }

  int timeStep = disPosEvent->GetSender()->GetTimeStep();

  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>(
    m_DataTreeNode->GetData() );

  if ( planarFigure != NULL )
  {
    // Get the Geometry2D of the window the user interacts with (for 2D point 
    // projection)
    mitk::BaseRenderer *renderer = stateEvent->GetEvent()->GetSender();
    const Geometry2D *projectionPlane = renderer->GetCurrentWorldGeometry2D();

    //// For reading on the points, Ids etc
    //mitk::CurveModel::PointSetType *pointSet = curveModel->GetPointSet( timeStep );
    //if ( pointSet == NULL )
    //{
    //  return 0.0;
    //}

    //int visualizationMode = CurveModel::VISUALIZATION_MODE_PLANAR;
    //if ( renderer != NULL )
    //{
    //  m_DataTreeNode->GetIntProperty( "VisualizationMode", visualizationMode, renderer );
    //}                                    

    //if ( visualizationMode == CurveModel::VISUALIZATION_MODE_PLANAR )
    //{
    //  // Check if mouse is near the SELECTED point of the CurveModel (if != -1)
    //  if ( curveModel->GetSelectedPointId() != -1 )
    //  {
    //    Point3D selectedPoint;
    //    pointSet->GetPoint( curveModel->GetSelectedPointId(), &selectedPoint );

    //    float maxDistance = m_Precision * m_Precision;
    //    if ( maxDistance == 0.0 ) { maxDistance = 0.000001; }

    //    float distance = selectedPoint.SquaredEuclideanDistanceTo( 
    //      disPosEvent->GetWorldPosition() );

    //    if ( distance < maxDistance )
    //    {
    //      returnValue = 1.0;
    //    }
    //  }
    //}
    //else if ( visualizationMode == CurveModel::VISUALIZATION_MODE_PROJECTION )
    //{
    //  // Check if mouse is near the PROJECTION  of any point of the CurveModel
    //  if ( curveModel->SearchPoint(
    //          disPosEvent->GetWorldPosition(), m_Precision, -1, projectionPlane, timeStep) > -1 ) 
    //  {
    //    returnValue = 1.0;
    //  }
    //}
  }
  return returnValue;
}


bool mitk::PlanarFigureInteractor
::ExecuteAction( Action *action, mitk::StateEvent const *stateEvent )
{
  bool ok = false;

  // Check corresponding data; has to be sub-class of mitk::PlanarFigure
  mitk::PlanarFigure *planarFigure = 
    dynamic_cast< mitk::PlanarFigure * >( m_DataTreeNode->GetData() );

  if ( planarFigure == NULL )
  {
    return false;
  }

  // Get the timestep to also support 3D+t
  const mitk::Event *theEvent = stateEvent->GetEvent();
  int timeStep = 0;
  mitk::ScalarType timeInMS = 0.0;

  if ( theEvent )
  {
    if (theEvent->GetSender() != NULL)
    {
      timeStep = theEvent->GetSender()->GetTimeStep( planarFigure );
      timeInMS = theEvent->GetSender()->GetTime();
    }
  }

  // Get the Geometry2D of the window the user interacts with (for 2D point 
  // projection)
  mitk::BaseRenderer *renderer = NULL;
  const Geometry2D *projectionPlane = NULL;
  if ( theEvent )
  {
    renderer = theEvent->GetSender();
    projectionPlane = renderer->GetCurrentWorldGeometry2D();
  }


  switch (action->GetActionId())
  {
  case AcDONOTHING:
    ok = true;
    break;


  case AcCHECKPOINT:
    {
      mitk::PositionEvent const  *posEvent = 
        dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());

      if (posEvent != NULL)
      {
        mitk::Point3D worldPoint = posEvent->GetWorldPosition();
      }

      // In all other cases: new Event with information NO
      mitk::StateEvent *newStateEvent = 
        new mitk::StateEvent(EIDNO, posEvent);
      this->HandleEvent(newStateEvent );
      ok = true;

      break;
    }

  case AcSELECTPOINT:
    {
      ok = true;  
      break;
    }

  case AcDESELECTALL:
    {
      ok = true;
      break;
    }

  //case AcMOVEPOINT:
  case AcMOVESELECTED:
    {
      // Update the display
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      ok = true;
      break;
    }

  case AcFINISHMOVE:
    {
      ok = true;
      break;
    }




  default:
    return Superclass::ExecuteAction( action, stateEvent );
  }

  return ok;
}
