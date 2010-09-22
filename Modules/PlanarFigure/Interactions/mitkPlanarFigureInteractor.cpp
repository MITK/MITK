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
#include "mitkDataNode.h"
#include "mitkInteractionConst.h"
#include "mitkAction.h"
#include "mitkStateEvent.h"
#include "mitkOperationEvent.h"
#include "mitkUndoController.h"
#include "mitkStateMachineFactory.h"
#include "mitkStateTransitionOperation.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateOR.h"


//how precise must the user pick the point
//default value
mitk::PlanarFigureInteractor
::PlanarFigureInteractor(const char * type, DataNode* dataNode, int /* n */ )
: Interactor( type, dataNode ),
  m_Precision( 6.5 ),
  m_IsHovering( false )
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
::CanHandleEvent(StateEvent const* stateEvent) const
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

  //int timeStep = disPosEvent->GetSender()->GetTimeStep();

  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>(
    m_DataNode->GetData() );

  if ( planarFigure != NULL )
  {
    // Give higher priority if this figure is currently selected
    if ( planarFigure->GetSelectedControlPoint() >= 0 )
    {
      return 1.0;
    }

    // Get the Geometry2D of the window the user interacts with (for 2D point 
    // projection)
    //mitk::BaseRenderer *renderer = stateEvent->GetEvent()->GetSender();
    //const Geometry2D *projectionPlane = renderer->GetCurrentWorldGeometry2D();

    //// For reading on the points, Ids etc
    //mitk::CurveModel::PointSetType *pointSet = curveModel->GetPointSet( timeStep );
    //if ( pointSet == NULL )
    //{
    //  return 0.0;
    //}

    //int visualizationMode = CurveModel::VISUALIZATION_MODE_PLANAR;
    //if ( renderer != NULL )
    //{
    //  m_DataNode->GetIntProperty( "VisualizationMode", visualizationMode, renderer );
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
    dynamic_cast< mitk::PlanarFigure * >( m_DataNode->GetData() );

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
      // Invoke event to notify listeners that placement of this PF starts now
      planarFigure->InvokeEvent( StartPlacementPlanarFigureEvent() );

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
      //this->LogPrintPlanarFigureQuantities( planarFigure );

      // Set a bool property indicating that the figure has been placed in
      // the current RenderWindow. This is required so that the same render
      // window can be re-aligned to the Geometry2D of the PlanarFigure later
      // on in an application.
      m_DataNode->SetBoolProperty( "PlanarFigureInitializedWindow", true, renderer );

      // Update rendered scene
      renderer->GetRenderingManager()->RequestUpdateAll();

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
      //this->LogPrintPlanarFigureQuantities( planarFigure );

      // Update rendered scene
      renderer->GetRenderingManager()->RequestUpdateAll();

      ok = true;
      break;
    }


  case AcCHECKNMINUS1:
    {
      if ( planarFigure->GetNumberOfControlPoints() >=
           planarFigure->GetMaximumNumberOfControlPoints() )
      {
        // Initial placement finished: deselect control point and send an
        // event to notify application listeners
        planarFigure->Modified();
        planarFigure->DeselectControlPoint();
        planarFigure->InvokeEvent( EndPlacementPlanarFigureEvent() );
        planarFigure->InvokeEvent( EndInteractionPlanarFigureEvent() );
        m_DataNode->Modified();
        this->HandleEvent( new mitk::StateEvent( EIDYES, stateEvent->GetEvent() ) );
      }
      else
      {
        this->HandleEvent( new mitk::StateEvent( EIDNO, stateEvent->GetEvent() ) );
      }

      // Update rendered scene
      renderer->GetRenderingManager()->RequestUpdateAll();

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
        // event to notify application listeners
        planarFigure->Modified();
        planarFigure->DeselectControlPoint();
        if ( planarFigure->GetNumberOfControlPoints()-1 >= planarFigure->GetMinimumNumberOfControlPoints() )
        {
          planarFigure->RemoveLastControlPoint();
        }
        planarFigure->InvokeEvent( EndPlacementPlanarFigureEvent() );
        planarFigure->InvokeEvent( EndInteractionPlanarFigureEvent() );
        m_DataNode->Modified();
        this->HandleEvent( new mitk::StateEvent( EIDYES, NULL ) );
      }
      else
      {
        this->HandleEvent( new mitk::StateEvent( EIDNO, NULL ) );
      }

      // Update rendered scene
      renderer->GetRenderingManager()->RequestUpdateAll();

      ok = true;
      break;
    }


  case AcCHECKPOINT:
    {
      // Check if the distance of the current point to the previously set point in display coordinates
      // is sufficient (if a previous point exists)

      // Extract display position
      const mitk::PositionEvent *positionEvent = 
        dynamic_cast< const mitk::PositionEvent * > ( stateEvent->GetEvent() );
      if ( positionEvent == NULL )
      {
        ok = false;
        break;
      }

      // Get current display position of the mouse
      mitk::Point2D currentDisplayPosition = positionEvent->GetDisplayPosition();

      // Check if a previous point has been set
      int previousIndex = planarFigure->GetNumberOfControlPoints() - 2;
      if ( previousIndex >= 0 )
      {
        
        // Try to convert previous point to current display coordinates
        mitk::Point3D previousPoint3D;
        planarFigureGeometry->Map( planarFigure->GetControlPoint( previousIndex ), previousPoint3D );
        if ( renderer->GetDisplayGeometry()->Distance( previousPoint3D ) < 0.1 )
        {
          mitk::Point2D previousDisplayPosition;
          renderer->GetCurrentWorldGeometry2D()->Map( previousPoint3D, previousDisplayPosition );
          renderer->GetDisplayGeometry()->WorldToDisplay( previousDisplayPosition, previousDisplayPosition );

          double a = currentDisplayPosition[0] - previousDisplayPosition[0];
          double b = currentDisplayPosition[1] - previousDisplayPosition[1];

          // If point is to close, do not set a new point
          if ( a * a + b * b < 25.0 )
          {
            this->HandleEvent( new mitk::StateEvent( EIDNO, stateEvent->GetEvent() ) );

            ok = true;
            break;
          }
        }
      }

      this->HandleEvent( new mitk::StateEvent( EIDYES, stateEvent->GetEvent() ) );    
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
      //this->LogPrintPlanarFigureQuantities( planarFigure );

      // Update rendered scene
      renderer->GetRenderingManager()->RequestUpdateAll();

      ok = true;
      break;
    }


  case AcDESELECTPOINT:
    {
      planarFigure->DeselectControlPoint();

      // Issue event so that listeners may update themselves
      planarFigure->Modified();
      planarFigure->InvokeEvent( EndInteractionPlanarFigureEvent() );

      m_DataNode->Modified();

      // falls through
    }

  case AcCHECKSELECTED:
    {
      bool isHovering = mitk::PlanarFigureInteractor::IsPositionOverFigure(
        stateEvent, planarFigure,
        planarFigureGeometry,
        renderer->GetCurrentWorldGeometry2D(),
        renderer->GetDisplayGeometry() );

      int pointIndex = mitk::PlanarFigureInteractor::IsPositionInsideMarker(
        stateEvent, planarFigure,
        planarFigureGeometry,
        renderer->GetCurrentWorldGeometry2D(),
        renderer->GetDisplayGeometry() );

      if ( pointIndex >= 0 )
      {
        // If mouse is above control point, mark it as selected
        planarFigure->SelectControlPoint( pointIndex );     

        // If mouse is hovering above a marker, it is also hovering above the figure
        isHovering = true;
      }
      else
      {
        // Mouse in not above control point --> deselect point
        planarFigure->DeselectControlPoint();
      }

      if ( isHovering )
      {
        if ( !m_IsHovering )
        {
          // Invoke hover event once when the mouse is entering the figure area
          m_IsHovering = true;
          planarFigure->InvokeEvent( StartHoverPlanarFigureEvent() );

          // Set bool property to indicate that planar figure is currently in "hovering" mode
          m_DataNode->SetBoolProperty( "planarfigure.hover", true );
        }

        this->HandleEvent( new mitk::StateEvent( EIDYES, NULL ) );

        // Return true: only this interactor is eligible to react on this event
        ok = true;
      }
      else
      {
        if ( m_IsHovering )
        {
          // Invoke end-hover event once the mouse is exiting the figure area
          m_IsHovering = false;
          planarFigure->InvokeEvent( EndHoverPlanarFigureEvent() );

          // Set bool property to indicate that planar figure is no longer in "hovering" mode
          m_DataNode->SetBoolProperty( "planarfigure.hover", false );
        }

        this->HandleEvent( new mitk::StateEvent( EIDNO, NULL ) );

        // Return false so that other (PlanarFigure) Interactors may react on this
        // event as well
        ok = false;
      }

      // Update rendered scene
       renderer->GetRenderingManager()->RequestUpdateAll();
      break;
    }

  case AcSELECTPICKEDOBJECT:
    {
      // Invoke event to notify listeners that this planar figure should be selected
      planarFigure->InvokeEvent( SelectPlanarFigureEvent() );

      int pointIndex = mitk::PlanarFigureInteractor::IsPositionInsideMarker(
        stateEvent, planarFigure,
        planarFigureGeometry,
        renderer->GetCurrentWorldGeometry2D(),
        renderer->GetDisplayGeometry() );

      if ( pointIndex >= 0 )
      {
        this->HandleEvent( new mitk::StateEvent( EIDYES, NULL ) );

        // Return true: only this interactor is eligible to react on this event
        ok = true;
      }
      else
      {
        this->HandleEvent( new mitk::StateEvent( EIDNO, NULL ) );

        // Return false so that other (PlanarFigure) Interactors may react on this
        // event as well
        ok = false;
      }

      ok = true;  
      break;
    }


  case AcSELECTPOINT:
    {
      // Invoke event to notify listeners that interaction with this PF starts now
      planarFigure->InvokeEvent( StartInteractionPlanarFigureEvent() );

      // Reset the PlanarFigure if required
      if ( planarFigure->ResetOnPointSelect() )
      {
        this->HandleEvent( new mitk::StateEvent( EIDYES, stateEvent->GetEvent() ) );
      }
      else
      {
        this->HandleEvent( new mitk::StateEvent( EIDNO, stateEvent->GetEvent() ) );
      }

      ok = true;  
      break;
    }

  //case AcMOVEPOINT:
  //case AcMOVESELECTED:
  //  {
  //    // Update the display
  //    renderer->GetRenderingManager()->RequestUpdateAll();

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


bool mitk::PlanarFigureInteractor::TransformObjectToDisplay(
  const mitk::Point2D &point2D,
  mitk::Point2D &displayPoint,
  const mitk::Geometry2D *objectGeometry,
  const mitk::Geometry2D *rendererGeometry,
  const mitk::DisplayGeometry *displayGeometry ) const
{
  mitk::Point3D point3D;

  // Map circle point from local 2D geometry into 3D world space
  objectGeometry->Map( point2D, point3D );

  // TODO: proper handling of distance tolerance
  if ( displayGeometry->Distance( point3D ) < 0.1 )
  {
    // Project 3D world point onto display geometry
    rendererGeometry->Map( point3D, displayPoint );
    displayGeometry->WorldToDisplay( displayPoint, displayPoint );
    return true;
  }

  return false;
}


bool mitk::PlanarFigureInteractor::IsPointNearLine(
  const mitk::Point2D& point,
  const mitk::Point2D& startPoint, const mitk::Point2D& endPoint ) const
{
  mitk::Vector2D n1 = endPoint - startPoint;
  n1.Normalize();

  // Determine dot products between line vector and startpoint-point / endpoint-point vectors
  double l1 = n1 * (point - startPoint);
  double l2 = -n1 * (point - endPoint);

  // Determine projection of specified point onto line defined by start / end point
  mitk::Point2D crossPoint = startPoint + n1 * l1;

  // Point is inside encompassing rectangle IF
  // - its distance to its projected point is small enough
  // - it is not further outside of the line than the defined tolerance
  if ( (crossPoint.SquaredEuclideanDistanceTo( point ) < 20.0 )
    && ( l1 > -5.0 ) && ( l2 > -5.0 ) )
  {
    return true;
  }

  return false;
}


bool mitk::PlanarFigureInteractor::IsPositionOverFigure(
  const StateEvent *stateEvent, PlanarFigure *planarFigure,
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

  mitk::Point2D displayPosition = positionEvent->GetDisplayPosition();


  // Iterate over all polylines of planar figure, and check if
  // any one is close to the current display position
  typedef mitk::PlanarFigure::VertexContainerType VertexContainerType;
  const VertexContainerType *controlPoints = planarFigure->GetControlPoints();

  mitk::Point2D worldPoint2D, displayControlPoint;
  mitk::Point3D worldPoint3D;

  for ( unsigned short loop = 0; loop < planarFigure->GetPolyLinesSize(); ++loop )
  {
    const VertexContainerType* polyLine = planarFigure->GetPolyLine( loop );

    Point2D polyLinePoint;
    Point2D firstPolyLinePoint;
    Point2D previousPolyLinePoint;

    bool firstPoint = true;
    for ( VertexContainerType::ConstIterator it = polyLine->Begin(); it != polyLine->End(); ++it )
    {
      // Get plane coordinates of this point of polyline (if possible)
      if ( !this->TransformObjectToDisplay( it->Value(), polyLinePoint,
        planarFigureGeometry, rendererGeometry, displayGeometry ) )
      {
        break; // Poly line invalid (not on current 2D plane) --> skip it
      }

      if ( firstPoint )
      {
        firstPolyLinePoint = polyLinePoint;
        firstPoint = false;
      }
      else if ( this->IsPointNearLine( displayPosition, previousPolyLinePoint, polyLinePoint ) )
      {
        // Return true if the display position is close enough to this line segment
        return true;
      }
      previousPolyLinePoint = polyLinePoint;
    }

    // For closed figures, also check last line segment
    if ( planarFigure->IsClosed()
      && this->IsPointNearLine( displayPosition, polyLinePoint, firstPolyLinePoint ) )
    {
      return true;
    }
  }

  return false;
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
    Point2D displayControlPoint;
    if ( this->TransformObjectToDisplay( it->Value(), displayControlPoint,
      planarFigureGeometry, rendererGeometry, displayGeometry ) )
    {
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
  MITK_INFO << "PlanarFigure: " << planarFigure->GetNameOfClass();
  for ( unsigned int i = 0; i < planarFigure->GetNumberOfFeatures(); ++i )
  {
    MITK_INFO << "* " << planarFigure->GetFeatureName( i ) << ": "
      << planarFigure->GetQuantity( i ) << " " << planarFigure->GetFeatureUnit( i );
  }
}

