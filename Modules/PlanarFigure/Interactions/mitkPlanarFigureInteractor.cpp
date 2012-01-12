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
#include "mitkNodePredicateOr.h"


//how precise must the user pick the point
//default value
mitk::PlanarFigureInteractor
::PlanarFigureInteractor(const char * type, DataNode* dataNode, int /* n */ )
: Interactor( type, dataNode ),
m_Precision( 6.5 ),
m_MinimumPointDistance( 25.0 ),
m_IsHovering( false ),
m_LastPointWasValid( false )
{
}

mitk::PlanarFigureInteractor::~PlanarFigureInteractor()
{
}

void mitk::PlanarFigureInteractor::SetPrecision( mitk::ScalarType precision )
{
  m_Precision = precision;
}


void mitk::PlanarFigureInteractor::SetMinimumPointDistance( ScalarType minimumDistance )
{
  m_MinimumPointDistance = minimumDistance;
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

  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>(
    m_DataNode->GetData() );

  if ( planarFigure != NULL )
  {
    // Give higher priority if this figure is currently selected
    if ( planarFigure->GetSelectedControlPoint() >= 0 )
    {
      return 1.0;
    }

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
      bool isEditable = true;
      m_DataNode->GetBoolProperty( "planarfigure.iseditable", isEditable );

      // Extract point in 2D world coordinates (relative to Geometry2D of
      // PlanarFigure)
      Point2D point2D;
      if ( !this->TransformPositionEventToPoint2D( stateEvent, point2D,
        planarFigureGeometry ) || !isEditable )
      {
        ok = false;
        break;
      }

      // check if the control points shall be hidden during interaction
      bool hidecontrolpointsduringinteraction = false;
      m_DataNode->GetBoolProperty( "planarfigure.hidecontrolpointsduringinteraction", hidecontrolpointsduringinteraction );

      // hide the control points if necessary
      m_DataNode->SetBoolProperty( "planarfigure.drawcontrolpoints", !hidecontrolpointsduringinteraction );

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
        planarFigure->SetProperty( "initiallyplaced", mitk::BoolProperty::New( true ) );
        m_DataNode->SetBoolProperty( "planarfigure.drawcontrolpoints", true );
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
      // be finished (by double-click)

      const mitk::PositionEvent *positionEvent =
        dynamic_cast< const mitk::PositionEvent * > ( stateEvent->GetEvent() );
      if ( positionEvent == NULL )
      {
        ok = false;
        break;
      }

      if ( m_LastPointWasValid && planarFigure->GetNumberOfControlPoints() > planarFigure->GetMinimumNumberOfControlPoints()  )
      {
        // Initial placement finished: deselect control point and send an
        // event to notify application listeners
        planarFigure->Modified();
        planarFigure->DeselectControlPoint();
        planarFigure->RemoveLastControlPoint();
        planarFigure->SetProperty( "initiallyplaced", mitk::BoolProperty::New( true ) );
        m_DataNode->SetBoolProperty( "planarfigure.drawcontrolpoints", true );
        m_DataNode->Modified();
        planarFigure->InvokeEvent( EndPlacementPlanarFigureEvent() );
        planarFigure->InvokeEvent( EndInteractionPlanarFigureEvent() );

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

      m_LastPointWasValid = IsMousePositionAcceptableAsNewControlPoint( positionEvent, planarFigure );
      if (m_LastPointWasValid)
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


  case AcADDPOINT:
    {
      bool selected = false;
      bool isEditable = true;
      m_DataNode->GetBoolProperty("selected", selected);
      m_DataNode->GetBoolProperty( "planarfigure.iseditable", isEditable );

      if ( !selected || !isEditable )
      {
        ok = false;
        break;
      }

      // Extract point in 2D world coordinates (relative to Geometry2D of
      // PlanarFigure)
      Point2D point2D, projectedPoint;
      if ( !this->TransformPositionEventToPoint2D( stateEvent, point2D,
        planarFigureGeometry ) )
      {
        ok = false;
        break;
      }

      // TODO: check segement of polyline we clicked in
      int nextIndex = this->IsPositionOverFigure(
        stateEvent, planarFigure,
        planarFigureGeometry,
        projectionPlane,
        renderer->GetDisplayGeometry(),
        projectedPoint
        );

      // Add point as new control point
      renderer->GetDisplayGeometry()->DisplayToWorld( projectedPoint, projectedPoint );

      if ( planarFigure->IsPreviewControlPointVisible() )
      {
        point2D = planarFigure->GetPreviewControlPoint();
      }

      planarFigure->AddControlPoint( point2D, nextIndex );

      if ( planarFigure->IsPreviewControlPointVisible() )
      {
        planarFigure->SelectControlPoint( nextIndex );
        planarFigure->ResetPreviewContolPoint();
      }

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

      m_DataNode->SetBoolProperty( "planarfigure.drawcontrolpoints", true );
      m_DataNode->Modified();

      // falls through
    }

  case AcCHECKHOVERING:
    {
      mitk::Point2D pointProjectedOntoLine;
      int previousControlPoint = mitk::PlanarFigureInteractor::IsPositionOverFigure(
        stateEvent, planarFigure,
        planarFigureGeometry,
        projectionPlane,
        renderer->GetDisplayGeometry(),
        pointProjectedOntoLine
        );
      bool isHovering = ( previousControlPoint != -1 );

      int pointIndex = mitk::PlanarFigureInteractor::IsPositionInsideMarker(
        stateEvent, planarFigure,
        planarFigureGeometry,
        projectionPlane,
        renderer->GetDisplayGeometry() );

      int initiallySelectedControlPoint = planarFigure->GetSelectedControlPoint();

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

      bool renderingUpdateNeeded = true;
      if ( isHovering )
      {
        if ( !m_IsHovering )
        {
          // Invoke hover event once when the mouse is entering the figure area
          m_IsHovering = true;
          planarFigure->InvokeEvent( StartHoverPlanarFigureEvent() );

          // Set bool property to indicate that planar figure is currently in "hovering" mode
          m_DataNode->SetBoolProperty( "planarfigure.ishovering", true );

          renderingUpdateNeeded = true;
        }

        bool selected = false;
        bool isExtendable = false;
        bool isEditable = true;
        m_DataNode->GetBoolProperty("selected", selected);
        m_DataNode->GetBoolProperty("planarfigure.isextendable", isExtendable);
        m_DataNode->GetBoolProperty( "planarfigure.iseditable", isEditable );

        if ( selected && isHovering && isExtendable && pointIndex == -1 && isEditable )
        {
          const mitk::PositionEvent *positionEvent =
            dynamic_cast< const mitk::PositionEvent * > ( stateEvent->GetEvent() );
          if ( positionEvent != NULL )
          {
            renderer->GetDisplayGeometry()->DisplayToWorld( pointProjectedOntoLine, pointProjectedOntoLine );
            planarFigure->SetPreviewControlPoint( pointProjectedOntoLine );

            renderingUpdateNeeded = true;
          }
        }
        else
        {
          planarFigure->ResetPreviewContolPoint();
        }

        if ( planarFigure->GetSelectedControlPoint() != initiallySelectedControlPoint  )
        {
          // the selected control point has changed -> rendering update necessary
          renderingUpdateNeeded = true;
        }

        this->HandleEvent( new mitk::StateEvent( EIDYES, stateEvent->GetEvent() ) );

        // Return true: only this interactor is eligible to react on this event
        ok = true;
      }
      else
      {
        if ( m_IsHovering )
        {
          planarFigure->ResetPreviewContolPoint();

          // Invoke end-hover event once the mouse is exiting the figure area
          m_IsHovering = false;
          planarFigure->InvokeEvent( EndHoverPlanarFigureEvent() );

          // Set bool property to indicate that planar figure is no longer in "hovering" mode
          m_DataNode->SetBoolProperty( "planarfigure.ishovering", false );

          renderingUpdateNeeded = true;
        }

        this->HandleEvent( new mitk::StateEvent( EIDNO, NULL ) );

        // Return false so that other (PlanarFigure) Interactors may react on this
        // event as well
        ok = false;
      }

      // Update rendered scene if necessray
      if ( renderingUpdateNeeded )
      {
        renderer->GetRenderingManager()->RequestUpdateAll();
      }
      break;
    }

  case AcCHECKSELECTED:
    {
      bool selected = false;
      m_DataNode->GetBoolProperty("selected", selected);

      if ( selected )
      {
        this->HandleEvent( new mitk::StateEvent( EIDYES, stateEvent->GetEvent() ) );
      }
      else
      {
        // Invoke event to notify listeners that this planar figure should be selected
        planarFigure->InvokeEvent( SelectPlanarFigureEvent() );
        this->HandleEvent( new mitk::StateEvent( EIDNO, NULL ) );
      }
    }

  case AcSELECTPICKEDOBJECT:
    {
      //// Invoke event to notify listeners that this planar figure should be selected
      //planarFigure->InvokeEvent( SelectPlanarFigureEvent() );

      // Check if planar figure is marked as "editable"
      bool isEditable = true;
      m_DataNode->GetBoolProperty( "planarfigure.iseditable", isEditable );

      int pointIndex = -1;

      if ( isEditable )
      {
        // If planar figure is editable, check if mouse is over a control point
        pointIndex = mitk::PlanarFigureInteractor::IsPositionInsideMarker(
          stateEvent, planarFigure,
          planarFigureGeometry,
          projectionPlane,
          renderer->GetDisplayGeometry() );
      }

      // If editing is enabled and the mouse is currently over a control point, select it
      if ( pointIndex >= 0 )
      {
        this->HandleEvent( new mitk::StateEvent( EIDYES, stateEvent->GetEvent() ) );

        // Return true: only this interactor is eligible to react on this event
        ok = true;
      }
      else
      {
        this->HandleEvent( new mitk::StateEvent( EIDNO, stateEvent->GetEvent() ) );

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

  case AcREMOVEPOINT:
    {
      bool isExtendable = false;
      m_DataNode->GetBoolProperty("planarfigure.isextendable", isExtendable);

      if ( isExtendable )
      {
        int selectedControlPoint = planarFigure->GetSelectedControlPoint();
        planarFigure->RemoveControlPoint( selectedControlPoint );

        // Re-evaluate features
        planarFigure->EvaluateFeatures();
        planarFigure->Modified();

        m_DataNode->SetBoolProperty( "planarfigure.drawcontrolpoints", true );
        planarFigure->InvokeEvent( EndInteractionPlanarFigureEvent() );
        renderer->GetRenderingManager()->RequestUpdateAll();
        this->HandleEvent( new mitk::StateEvent( EIDYES, NULL ) );
      }
      else
      {
        this->HandleEvent( new mitk::StateEvent( EIDNO, NULL ) );
      }
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
  const mitk::Point2D& startPoint, 
  const mitk::Point2D& endPoint,
  mitk::Point2D& projectedPoint
  ) const
{
  mitk::Vector2D n1 = endPoint - startPoint;
  n1.Normalize();

  // Determine dot products between line vector and startpoint-point / endpoint-point vectors
  double l1 = n1 * (point - startPoint);
  double l2 = -n1 * (point - endPoint);

  // Determine projection of specified point onto line defined by start / end point
  mitk::Point2D crossPoint = startPoint + n1 * l1;
  projectedPoint = crossPoint;

  // Point is inside encompassing rectangle IF
  // - its distance to its projected point is small enough
  // - it is not further outside of the line than the defined tolerance
  if (((crossPoint.SquaredEuclideanDistanceTo(point) < 20.0) && (l1 > 0.0) && (l2 > 0.0))
      || endPoint.SquaredEuclideanDistanceTo(point) < 20.0 
      || startPoint.SquaredEuclideanDistanceTo(point) < 20.0)
  {
    return true;
  }

  return false;
}


int mitk::PlanarFigureInteractor::IsPositionOverFigure(
  const StateEvent *stateEvent, PlanarFigure *planarFigure,
  const Geometry2D *planarFigureGeometry,
  const Geometry2D *rendererGeometry,
  const DisplayGeometry *displayGeometry,
  Point2D& pointProjectedOntoLine ) const
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
  typedef mitk::PlanarFigure::PolyLineType VertexContainerType;

  mitk::Point2D worldPoint2D, displayControlPoint;
  mitk::Point3D worldPoint3D;

  for ( unsigned short loop = 0; loop < planarFigure->GetPolyLinesSize(); ++loop )
  {
    const VertexContainerType polyLine = planarFigure->GetPolyLine( loop );

    Point2D polyLinePoint;
    Point2D firstPolyLinePoint;
    Point2D previousPolyLinePoint;

    bool firstPoint = true;
    for ( VertexContainerType::const_iterator it = polyLine.begin(); it != polyLine.end(); ++it )
    {
      // Get plane coordinates of this point of polyline (if possible)
      if ( !this->TransformObjectToDisplay( it->Point, polyLinePoint,
        planarFigureGeometry, rendererGeometry, displayGeometry ) )
      {
        break; // Poly line invalid (not on current 2D plane) --> skip it
      }

      if ( firstPoint )
      {
        firstPolyLinePoint = polyLinePoint;
        firstPoint = false;
      }
      else if ( this->IsPointNearLine( displayPosition, previousPolyLinePoint, polyLinePoint, pointProjectedOntoLine ) )
      {
        // Point is close enough to line segment --> Return index of the segment
        return it->Index;
      }
      previousPolyLinePoint = polyLinePoint;
    }

    // For closed figures, also check last line segment
    if ( planarFigure->IsClosed()
      && this->IsPointNearLine( displayPosition, polyLinePoint, firstPolyLinePoint, pointProjectedOntoLine ) )
    {
      return 0; // Return index of first control point
    }
  }

  return -1;
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
  mitk::Point2D worldPoint2D, displayControlPoint;
  mitk::Point3D worldPoint3D;

  int numberOfControlPoints = planarFigure->GetNumberOfControlPoints();
  for ( int i=0; i<numberOfControlPoints; i++ )
  {
    Point2D displayControlPoint;
    if ( this->TransformObjectToDisplay( planarFigure->GetControlPoint(i), displayControlPoint,
      planarFigureGeometry, rendererGeometry, displayGeometry ) )
    {
      // TODO: variable size of markers
      if ( displayPosition.SquaredEuclideanDistanceTo( displayControlPoint ) < 20.0 )
      {
        return i;
      }
    }
  }


  //for ( it = controlPoints.begin(); it != controlPoints.end(); ++it )
  //{
  //  Point2D displayControlPoint;
  //  if ( this->TransformObjectToDisplay( it->Point, displayControlPoint,
  //    planarFigureGeometry, rendererGeometry, displayGeometry ) )
  //  {
  //    // TODO: variable size of markers
  //    if ( (abs(displayPosition[0] - displayControlPoint[0]) < 4 )
  //      && (abs(displayPosition[1] - displayControlPoint[1]) < 4 ) )
  //    {
  //      return index;
  //    }
  //  }
  //}

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

bool
mitk::PlanarFigureInteractor::IsMousePositionAcceptableAsNewControlPoint(
    const PositionEvent* positionEvent,
    const PlanarFigure* planarFigure )
{
  assert(positionEvent && planarFigure);

  BaseRenderer* renderer = positionEvent->GetSender();

  assert(renderer);

  // Get the timestep to support 3D+t
  int timeStep( renderer->GetTimeStep( planarFigure ) );

  // Get current display position of the mouse
  Point2D currentDisplayPosition = positionEvent->GetDisplayPosition();

  // Check if a previous point has been set
  bool tooClose = false;

  for( int i=0; i < (int)planarFigure->GetNumberOfControlPoints(); i++ )
  {
    if ( i != planarFigure->GetSelectedControlPoint() )
    {
      // Try to convert previous point to current display coordinates
      mitk::Geometry2D *planarFigureGeometry =
        dynamic_cast< mitk::Geometry2D * >( planarFigure->GetGeometry( timeStep ) );

      const Geometry2D *projectionPlane = renderer->GetCurrentWorldGeometry2D();

      mitk::Point3D previousPoint3D;
      planarFigureGeometry->Map( planarFigure->GetControlPoint( i ), previousPoint3D );
      if ( renderer->GetDisplayGeometry()->Distance( previousPoint3D ) < 0.1 ) // ugly, but assert makes this work
      {
        mitk::Point2D previousDisplayPosition;
        projectionPlane->Map( previousPoint3D, previousDisplayPosition );
        renderer->GetDisplayGeometry()->WorldToDisplay( previousDisplayPosition, previousDisplayPosition );

        double a = currentDisplayPosition[0] - previousDisplayPosition[0];
        double b = currentDisplayPosition[1] - previousDisplayPosition[1];

        // If point is to close, do not set a new point
        tooClose = (a * a + b * b < m_MinimumPointDistance );
      }
      if ( tooClose )
        return false; // abort loop early
    }
  }

  return !tooClose; // default
}
