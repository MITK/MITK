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
  float returnValue = 0.5;
  

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
  //if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
  //{
  //  returnValue = 0.5;//it can be understood
  //}

  int timeStep = disPosEvent->GetSender()->GetTimeStep();

  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>(
    m_DataTreeNode->GetData() );

  if ( planarFigure != NULL )
  {
    // Give higher priority if this figure is currently selected
    if ( planarFigure->GetSelectedControlPoint() >= 0 )
    {
      return 1.0;
    }

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

  // Get Geometry2D of PlanarFigure
  mitk::Geometry2D *planarFigureGeometry =
    dynamic_cast< mitk::Geometry2D * >( planarFigure->GetGeometry( timeStep ) );

  // Get the Geometry2D of the window the user interacts with (for 2D point 
  // projection)
  mitk::BaseRenderer *renderer = NULL;
  const Geometry2D *projectionPlane = NULL;
  if ( theEvent )
  {
    renderer = theEvent->GetSender();
    projectionPlane = renderer->GetCurrentWorldGeometry2D();
  }

  // TODO: Check if display and PlanarFigure geometries are parallel (if they are PlaneGeometries)


  switch (action->GetActionId())
  {
  case AcDONOTHING:
    ok = true;
    break;

  case AcCHECKOBJECT:
    {
      if ( planarFigure->IsPlaced() )
      {
        this->HandleEvent( new mitk::StateEvent( EIDYES, NULL ) );
      }
      else
      {
        this->HandleEvent( new mitk::StateEvent( EIDNO, NULL ) );
      }
      ok = false;
      break;
    }

  case AcADD:
    {
      // Use Geometry2D of the renderer clicked on for this PlanarFigure
      mitk::PlaneGeometry *planeGeometry = const_cast< mitk::PlaneGeometry * >(
        dynamic_cast< const mitk::PlaneGeometry * >(
          renderer->GetSliceNavigationController()->GetCurrentPlaneGeometry() ) );
      if ( planeGeometry != NULL )
      {
        planarFigureGeometry = planeGeometry;
        planarFigure->SetGeometry2D( planeGeometry );
      }
      else
      {
        ok = false;
        break;
      }

      // Extract point in 2D world coordinates (relative to Geometry2D of
      // PlanarFigure)
      Point2D point2D;
      if ( !this->TransformPositionEventToPoint2D( stateEvent, point2D,
        planarFigureGeometry ) )
      {
        ok = false;
        break;
      }

      // Place PlanarFigure at this point
      planarFigure->PlaceFigure( point2D );

      // Re-evaluate features
      planarFigure->EvaluateFeatures();
      this->LogPrintPlanarFigureQuantities( planarFigure );

      // Set a bool property indicating that the figure has been placed in
      // the current RenderWindow. This is required so that the same render
      // window can be re-aligned to the Geometry2D of the PlanarFigure later
      // on in an application.
      m_DataTreeNode->SetBoolProperty( "PlanarFigureInitializedWindow", true, renderer );

      // Update rendered scene
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      ok = true;
      break;
    }

  case AcMOVEPOINT:
    {
      // Extract point in 2D world coordinates (relative to Geometry2D of
      // PlanarFigure)
      Point2D point2D;
      if ( !this->TransformPositionEventToPoint2D( stateEvent, point2D,
        planarFigureGeometry ) )
      {
        ok = false;
        break;
      }

      // Move current control point to this point
      planarFigure->SetCurrentControlPoint( point2D );

      // Re-evaluate features
      planarFigure->EvaluateFeatures();
      this->LogPrintPlanarFigureQuantities( planarFigure );

      // Update rendered scene
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      ok = true;
      break;
    }


  case AcCHECKNMINUS1:
    {
      if ( planarFigure->GetNumberOfControlPoints() >=
           planarFigure->GetMaximumNumberOfControlPoints() )
      {
        // Initial placement finished: deselect control point and send an
        // InitializeEvent to notify application listeners
        planarFigure->Modified();
        planarFigure->DeselectControlPoint();
        planarFigure->InvokeEvent( itk::InitializeEvent() );
        planarFigure->InvokeEvent( itk::EndEvent() );
        this->HandleEvent( new mitk::StateEvent( EIDYES, stateEvent->GetEvent() ) );
      }
      else
      {
        this->HandleEvent( new mitk::StateEvent( EIDNO, stateEvent->GetEvent() ) );
      }

      // Update rendered scene
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      ok = true;
      break;
    }


  case AcCHECKEQUALS1:
    {
      // NOTE: Action name is a bit misleading; this action checks whether
      // the figure has already the minimum number of required points to
      // be finished.

      if ( planarFigure->GetNumberOfControlPoints() >=
        planarFigure->GetMinimumNumberOfControlPoints() )
      {
        // Initial placement finished: deselect control point and send an
        // InitializeEvent to notify application listeners
        planarFigure->Modified();
        planarFigure->DeselectControlPoint();
        planarFigure->InvokeEvent( itk::InitializeEvent() );
        planarFigure->InvokeEvent( itk::EndEvent() );
        this->HandleEvent( new mitk::StateEvent( EIDYES, NULL ) );
      }
      else
      {
        this->HandleEvent( new mitk::StateEvent( EIDNO, NULL ) );
      }

      // Update rendered scene
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      ok = true;
      break;
    }


  case AcADDPOINT:
    {
      // Extract point in 2D world coordinates (relative to Geometry2D of
      // PlanarFigure)
      Point2D point2D;
      if ( !this->TransformPositionEventToPoint2D( stateEvent, point2D,
        planarFigureGeometry ) )
      {
        ok = false;
        break;
      }

      // Add point as new control point
      planarFigure->AddControlPoint( point2D );

      // Re-evaluate features
      planarFigure->EvaluateFeatures();
      this->LogPrintPlanarFigureQuantities( planarFigure );

      // Update rendered scene
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      ok = true;
      break;
    }


  case AcDESELECTPOINT:
    {
      planarFigure->DeselectControlPoint();

      // Issue event so that listeners may update themselves
      planarFigure->Modified();
      planarFigure->InvokeEvent( itk::EndEvent() );

      // falls through
    }

  case AcCHECKPOINT:
    {
      int pointIndex = mitk::PlanarFigureInteractor::IsPositionInsideMarker(
        stateEvent, planarFigure,
        planarFigureGeometry,
        renderer->GetCurrentWorldGeometry2D(),
        renderer->GetDisplayGeometry() );

      if ( pointIndex >= 0 )
      {
        planarFigure->SelectControlPoint( pointIndex );
        this->HandleEvent( new mitk::StateEvent( EIDYES, NULL ) );

        // Return true: only this interactor is eligible to react on this event
        ok = true;
      }
      else
      {
        planarFigure->DeselectControlPoint();
        this->HandleEvent( new mitk::StateEvent( EIDNO, NULL ) );

        // Return false so that other (PlanarFigure) Interactors may react on this
        // event as well
        ok = false;
      }

      // Update rendered scene
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      break;
    }

  case AcSELECTPOINT:
    {
      ok = true;  
      break;
    }

  //case AcMOVEPOINT:
  //case AcMOVESELECTED:
  //  {
  //    // Update the display
  //    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  //    ok = true;
  //    break;
  //  }

  //case AcFINISHMOVE:
  //  {
  //    ok = true;
  //    break;
  //  }




  default:
    return Superclass::ExecuteAction( action, stateEvent );
  }

  return ok;
}

bool mitk::PlanarFigureInteractor::TransformPositionEventToPoint2D(
  const StateEvent *stateEvent, Point2D &point2D,
  const Geometry2D *planarFigureGeometry )
{
  // Extract world position, and from this position on geometry, if
  // available
  const mitk::PositionEvent *positionEvent = 
    dynamic_cast< const mitk::PositionEvent * > ( stateEvent->GetEvent() );
  if ( positionEvent == NULL )
  {
    return false;
  }

  mitk::Point3D worldPoint3D = positionEvent->GetWorldPosition();

  // TODO: proper handling of distance tolerance
  if ( planarFigureGeometry->Distance( worldPoint3D ) > 0.1 )
  {
    return false;
  }

  // Project point onto plane of this PlanarFigure
  planarFigureGeometry->Map( worldPoint3D, point2D );
  return true;
}

int mitk::PlanarFigureInteractor::IsPositionInsideMarker(
  const StateEvent *stateEvent, const PlanarFigure *planarFigure,
  const Geometry2D *planarFigureGeometry,
  const Geometry2D *rendererGeometry,
  const DisplayGeometry *displayGeometry ) const
{
  // Extract display position
  const mitk::PositionEvent *positionEvent = 
    dynamic_cast< const mitk::PositionEvent * > ( stateEvent->GetEvent() );
  if ( positionEvent == NULL )
  {
    return -1;
  }

  //mitk::Point2D displayPosition;
  //mitk::Point3D cursorWorldPosition = positionEvent->GetWorldPosition();

  //displayGeometry->Project( cursorWorldPosition, cursorWorldPosition );
  //displayGeometry->Map( cursorWorldPosition, displayPosition );

  mitk::Point2D displayPosition = positionEvent->GetDisplayPosition();


  // Iterate over all control points of planar figure, and check if
  // any one is close to the current display position
  typedef mitk::PlanarFigure::VertexContainerType VertexContainerType;
  const VertexContainerType *controlPoints = planarFigure->GetControlPoints();

  mitk::Point2D worldPoint2D, displayControlPoint;
  mitk::Point3D worldPoint3D;

  VertexContainerType::ConstIterator it;
  for ( it = controlPoints->Begin(); it != controlPoints->End(); ++it )
  {
    planarFigureGeometry->Map( it->Value(), worldPoint3D );

    // TODO: proper handling of distance tolerance
    if ( displayGeometry->Distance( worldPoint3D ) < 0.1 )
    {
      rendererGeometry->Map( worldPoint3D, displayControlPoint );
      displayGeometry->WorldToDisplay( displayControlPoint, displayControlPoint );

      // TODO: variable size of markers
      if ( (abs(displayPosition[0] - displayControlPoint[0]) < 4 )
        && (abs(displayPosition[1] - displayControlPoint[1]) < 4 ) )
      {
        return it->Index();
      }
    }
  }

  return -1;
}


void mitk::PlanarFigureInteractor::LogPrintPlanarFigureQuantities( 
  const PlanarFigure *planarFigure )
{
  LOG_INFO << "PlanarFigure: " << planarFigure->GetNameOfClass();
  for ( unsigned int i = 0; i < planarFigure->GetNumberOfFeatures(); ++i )
  {
    LOG_INFO << "* " << planarFigure->GetFeatureName( i ) << ": "
      << planarFigure->GetQuantity( i ) << " " << planarFigure->GetFeatureUnit( i );
  }
}
