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


#define PLANARFIGUREINTERACTOR_DBG MITK_DEBUG("PlanarFigureInteractor") << __LINE__ << ": "

#include "mitkPlanarFigureInteractor.h"
#include "mitkPlanarFigure.h"
#include "mitkPlanarPolygon.h"
#include "mitkPlanarFigureOperation.h"
#include "mitkPlanarCircle.h"

#include "mitkInteractionPositionEvent.h"
#include "mitkInternalEvent.h"
#include "mitkInteractionConst.h"

#include "mitkOperationEvent.h"
#include "mitkUndoController.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"

#include "mitkPlaneGeometry.h"
#include "mitkAbstractTransformGeometry.h"

#include <memory>


//how precise must the user pick the point
//default value
mitk::PlanarFigureInteractor::PlanarFigureInteractor()
: DataInteractor()
, m_Precision( 6.5 )
, m_MinimumPointDistance( 25.0 )
, m_IsHovering( false )
, m_LastPointWasValid( false )
, m_PointMoved(false)
{
}

mitk::PlanarFigureInteractor::~PlanarFigureInteractor()
{
    RemoveObservers();
}

void mitk::PlanarFigureInteractor::ConnectActionsAndFunctions()
{
  CONNECT_CONDITION("figure_is_on_current_slice", CheckFigureOnRenderingGeometry);
  CONNECT_CONDITION("figure_is_placed", CheckFigurePlaced);
  CONNECT_CONDITION("minimal_figure_is_finished", CheckMinimalFigureFinished);
  CONNECT_CONDITION("hovering_above_figure", CheckFigureHovering);
  CONNECT_CONDITION("hovering_above_point", CheckControlPointHovering);
  CONNECT_CONDITION("figure_is_selected", CheckSelection);
  CONNECT_CONDITION("point_is_valid", CheckPointValidity);
  CONNECT_CONDITION("figure_is_finished", CheckFigureFinished);
  CONNECT_CONDITION("reset_on_point_select_needed", CheckResetOnPointSelect);
  CONNECT_CONDITION("points_can_be_added_or_removed", CheckFigureIsExtendable);


  CONNECT_FUNCTION( "finalize_figure", FinalizeFigure);
  CONNECT_FUNCTION( "hide_preview_point", HidePreviewPoint )
  CONNECT_FUNCTION( "hide_control_points", HideControlPoints )
  CONNECT_FUNCTION( "set_preview_point_position", SetPreviewPointPosition )
  CONNECT_FUNCTION( "move_current_point", MoveCurrentPoint);
  CONNECT_FUNCTION( "deselect_point", DeselectPoint);
  CONNECT_FUNCTION( "add_new_point", AddPoint);
  CONNECT_FUNCTION( "add_initial_point", AddInitialPoint);
  CONNECT_FUNCTION( "remove_selected_point", RemoveSelectedPoint);
  CONNECT_FUNCTION( "request_context_menu", RequestContextMenu);
  CONNECT_FUNCTION( "select_figure", SelectFigure );
  CONNECT_FUNCTION( "select_point", SelectPoint );
  CONNECT_FUNCTION( "end_interaction", EndInteraction );
  CONNECT_FUNCTION( "start_hovering", StartHovering )
  CONNECT_FUNCTION( "end_hovering", EndHovering );
  CONNECT_FUNCTION( "delete_figure", DeleteFigure );
}

void mitk::PlanarFigureInteractor::RemoveObservers()
{
    for (size_t i = 0; i < m_ObserverTagInfo.observerTags.size(); ++i) {
        m_ObserverTagInfo.object->RemoveObserver(m_ObserverTagInfo.observerTags[i]);
    }
    m_ObserverTagInfo.object = nullptr;
    m_ObserverTagInfo.observerTags.clear();
}

void mitk::PlanarFigureInteractor::DataNodeChanged()
{
    RemoveObservers();

    // Listen to change events
    if (GetDataNode()) {
        m_ObserverTagInfo.object = GetDataNode()->GetData();

        // Subscribe to change events
        itk::SimpleMemberCommand<PlanarFigureInteractor>::Pointer cancelPlaceCommand = itk::SimpleMemberCommand<PlanarFigureInteractor>::New();
        cancelPlaceCommand->SetCallbackFunction(this, &PlanarFigureInteractor::ProcessExternalCancelPlace);
        m_ObserverTagInfo.observerTags.push_back(m_ObserverTagInfo.object->AddObserver(CancelPlacementPlanarFigureEvent(), cancelPlaceCommand));

        itk::SimpleMemberCommand<PlanarFigureInteractor>::Pointer placeCommand = itk::SimpleMemberCommand<PlanarFigureInteractor>::New();
        placeCommand->SetCallbackFunction(this, &PlanarFigureInteractor::ProcessExternalPlace);
        m_ObserverTagInfo.observerTags.push_back(m_ObserverTagInfo.object->AddObserver(EndPlacementPlanarFigureEvent(), placeCommand));
    }
}

void mitk::PlanarFigureInteractor::ProcessExternalCancelPlace()
{
    mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>(GetDataNode()->GetData());
    if (!planarFigure->IsPlaced()) {
        if (HandleEvent(mitk::InternalEvent::New(nullptr, this, "Undo-Placement-Event"), GetDataNode())) {
//             planarFigure->GetPropertyList()->SetBoolProperty("initiallyplaced", false);
        }
    }
}

void mitk::PlanarFigureInteractor::ProcessExternalPlace()
{
    mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>(GetDataNode()->GetData());
    if (planarFigure->IsPlaced()) {
        if (HandleEvent(mitk::InternalEvent::New(nullptr, this, "Placement-Event"), GetDataNode())) {
//             planarFigure->GetPropertyList()->SetBoolProperty("initiallyplaced", true);
        }
    }
}

bool mitk::PlanarFigureInteractor::CheckFigurePlaced( const InteractionEvent* /*interactionEvent*/ )
{
  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>( GetDataNode()->GetData() );

//   bool isFigureFinished = false;
//   planarFigure->GetPropertyList()->GetBoolProperty( "initiallyplaced", isFigureFinished );
// 
  return planarFigure->IsPlaced() && planarFigure->IsFinalized();
}

bool mitk::PlanarFigureInteractor::MoveCurrentPoint(StateMachineAction*, InteractionEvent* interactionEvent)
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  if ( positionEvent == nullptr )
    return false;

  bool isEditable = true;
  GetDataNode()->GetBoolProperty( "planarfigure.iseditable", isEditable );

  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>(
    GetDataNode()->GetData() );

  mitk::PlaneGeometry *planarFigureGeometry =
    dynamic_cast< PlaneGeometry * >( planarFigure->GetGeometry( 0 ) );
  mitk::AbstractTransformGeometry *abstractTransformGeometry =
    dynamic_cast< AbstractTransformGeometry * >( planarFigure->GetGeometry( 0 ) );

  if ( abstractTransformGeometry != nullptr )
    return false;

  // Extract point in 2D world coordinates (relative to PlaneGeometry of
  // PlanarFigure)
  Point2D point2D;
  if ( !this->TransformPositionEventToPoint2D( positionEvent, planarFigureGeometry, point2D ) || !isEditable )
  {
    return false;
  }

  planarFigure->InvokeEvent( StartInteractionPlanarFigureEvent() );

  // check if the control points shall be hidden during interaction
  bool hidecontrolpointsduringinteraction = false;
  GetDataNode()->GetBoolProperty( "planarfigure.hidecontrolpointsduringinteraction", hidecontrolpointsduringinteraction );

  // hide the control points if necessary
  //interactionEvent->GetSender()->GetDataStorage()->BlockNodeModifiedEvents( true );
  GetDataNode()->SetBoolProperty( "planarfigure.drawcontrolpoints", !hidecontrolpointsduringinteraction );
  //interactionEvent->GetSender()->GetDataStorage()->BlockNodeModifiedEvents( false );

  // Move current control point to this point
  planarFigure->SetCurrentControlPoint( point2D );
  m_PointMoved = true;

  // Update rendered scene
  interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();

  return true;
}

void mitk::PlanarFigureInteractor::FinalizeFigure()
{
    mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>(GetDataNode()->GetData());

    //////////////////////////////////////////////////////////////////////////
    // Remove last control point
    int pointsBefore = planarFigure->GetNumberOfControlPoints();
    planarFigure->RemoveLastControlPoint();
    int pointsAfter = planarFigure->GetNumberOfControlPoints();

    if (m_UndoEnabled && pointsAfter != pointsBefore) {
        // Record the point removal
        int index = pointsAfter;

        std::unique_ptr<mitk::PlanarFigureOperation> doOp(new mitk::PlanarFigureOperation(
            OpREMOVE, mitk::Point2D(), index));

        std::unique_ptr<mitk::PlanarFigureOperation> undoOp(new mitk::PlanarFigureOperation(
            OpINSERT, mitk::Point2D(), index));

        OperationEvent *operationEvent = new OperationEvent(planarFigure, doOp.release(), undoOp.release(), "Remove point");
        m_UndoController->SetOperationEvent(operationEvent);
    }

    if (m_UndoEnabled && pointsAfter == pointsBefore && m_PointMoved)
    {
        // No points removed meaning placement is finished, but the last point was moved. Record the move event.
        int index = planarFigure->GetSelectedControlPoint();

        std::unique_ptr<mitk::PlanarFigureOperation> doOp(new mitk::PlanarFigureOperation(
            OpMOVE, planarFigure->GetControlPoint(index), index));

        std::unique_ptr<mitk::PlanarFigureOperation> undoOp(new mitk::PlanarFigureOperation(
            OpMOVE, m_StartMovePosition, index));

        OperationEvent *operationEvent = new OperationEvent(planarFigure, doOp.release(), undoOp.release(), "Move point");
        m_UndoController->SetOperationEvent(operationEvent);
    }

    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // Finalize figure
    std::unique_ptr<mitk::PlanarFigureOperation> finalizeOp(new mitk::PlanarFigureOperation(
        OpCLOSECELL, mitk::Point2D()));

    planarFigure->ExecuteOperation(finalizeOp.get());

    if (m_UndoEnabled)
    {
        std::unique_ptr<mitk::PlanarFigureOperation> undoFinalizeOp(new mitk::PlanarFigureOperation(
            OpOPENCELL, mitk::Point2D()));

        OperationEvent *operationEvent = new OperationEvent(planarFigure, finalizeOp.release(), undoFinalizeOp.release(), "Finalize figure");
        m_UndoController->SetOperationEvent(operationEvent);
    }

    planarFigure->DeselectControlPoint();

    //////////////////////////////////////////////////////////////////////////

    GetDataNode()->SetBoolProperty("planarfigure.drawcontrolpoints", true);
    planarFigure->InvokeEvent(EndPlacementPlanarFigureEvent());
    planarFigure->InvokeEvent(EndInteractionPlanarFigureEvent());
}

bool mitk::PlanarFigureInteractor::FinalizeFigure( StateMachineAction*, InteractionEvent* interactionEvent )
{
    FinalizeFigure();
  interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();

  return false;
}

bool mitk::PlanarFigureInteractor::EndInteraction( StateMachineAction*, InteractionEvent* interactionEvent )
{
  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>( GetDataNode()->GetData() );
  GetDataNode()->SetBoolProperty( "planarfigure.drawcontrolpoints", true );
  planarFigure->InvokeEvent( EndInteractionPlanarFigureEvent() );
  interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();

  return false;
}

bool mitk::PlanarFigureInteractor::EndHovering( StateMachineAction*, InteractionEvent* interactionEvent )
{
  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>( GetDataNode()->GetData() );
  planarFigure->ResetPreviewContolPoint();

  // Invoke end-hover event once the mouse is exiting the figure area
  m_IsHovering = false;
  planarFigure->InvokeEvent( EndHoverPlanarFigureEvent() );

  // Set bool property to indicate that planar figure is no longer in "hovering" mode
  GetDataNode()->SetBoolProperty( "planarfigure.ishovering", false );

  interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();

  return false;
}

bool mitk::PlanarFigureInteractor::DeleteFigure( StateMachineAction*, InteractionEvent* interactionEvent )
{
  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>( GetDataNode()->GetData() );
  planarFigure->RemoveAllObservers();
  GetDataNode()->RemoveAllObservers();

  interactionEvent->GetSender()->GetDataStorage()->Remove( GetDataNode() );
  interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();

  return false;
}

bool mitk::PlanarFigureInteractor::CheckMinimalFigureFinished( const InteractionEvent* /*interactionEvent*/ )
{
  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>( GetDataNode()->GetData() );
  return ( planarFigure->GetNumberOfControlPoints() >= planarFigure->GetMinimumNumberOfControlPoints()  );
}

bool mitk::PlanarFigureInteractor::CheckFigureFinished( const InteractionEvent* /*interactionEvent*/ )
{
  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>( GetDataNode()->GetData() );
  return ( planarFigure->GetNumberOfControlPoints() >= planarFigure->GetMaximumNumberOfControlPoints() );
}


bool mitk::PlanarFigureInteractor::CheckFigureIsExtendable( const InteractionEvent* /*interactionEvent*/ )
{
  bool isExtendable = false;
  GetDataNode()->GetBoolProperty("planarfigure.isextendable", isExtendable);

  return isExtendable;
}

bool mitk::PlanarFigureInteractor::DeselectPoint(StateMachineAction*, InteractionEvent* /*interactionEvent*/)
{
 mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>(
 GetDataNode()->GetData() );

  int index = planarFigure->GetSelectedControlPoint();
  bool wasSelected = planarFigure->DeselectControlPoint();
  if ( wasSelected )
  {
    // Issue event so that listeners may update themselves
    planarFigure->InvokeEvent( EndInteractionPlanarFigureEvent() );

    GetDataNode()->SetBoolProperty( "planarfigure.drawcontrolpoints", true );
//    GetDataNode()->SetBoolProperty( "planarfigure.ishovering", false );

    if (m_PointMoved) {
        m_PointMoved = false;
        mitk::OperationEvent::IncCurrGroupEventId();
        mitk::OperationEvent::IncCurrObjectEventId();
        mitk::OperationEvent::ExecuteIncrement();

        if (m_UndoEnabled)
        {
            std::unique_ptr<mitk::PlanarFigureOperation> doOp(new mitk::PlanarFigureOperation(
                OpMOVE, planarFigure->GetControlPoint(index), index));

            std::unique_ptr<mitk::PlanarFigureOperation> undoOp(new mitk::PlanarFigureOperation(
                OpMOVE, m_StartMovePosition, index));

            OperationEvent *operationEvent = new OperationEvent(planarFigure, doOp.release(), undoOp.release(), "Move point");
            m_UndoController->SetOperationEvent(operationEvent);
        } 
    }
  }

  return true;
}

bool mitk::PlanarFigureInteractor::AddPoint(StateMachineAction*, InteractionEvent* interactionEvent)
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  if ( positionEvent == nullptr )
    return false;

  bool selected = false;
  bool isEditable = true;
  GetDataNode()->GetBoolProperty("selected", selected);
  GetDataNode()->GetBoolProperty( "planarfigure.iseditable", isEditable );

  if ( !selected || !isEditable )
  {
    return false;
  }

  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>(
    GetDataNode()->GetData() );

  const mitk::PlaneGeometry *planarFigureGeometry = planarFigure->GetPlaneGeometry();
  mitk::AbstractTransformGeometry *abstractTransformGeometry =
    dynamic_cast< AbstractTransformGeometry * >( planarFigure->GetGeometry( 0 ) );

  if ( abstractTransformGeometry != nullptr)
    return false;

  // If the planarFigure already has reached the maximum number
  if ( planarFigure->GetNumberOfControlPoints() >= planarFigure->GetMaximumNumberOfControlPoints() )
  {
    return false;
  }

  // Extract point in 2D world coordinates (relative to PlaneGeometry of
  // PlanarFigure)
  Point2D point2D, projectedPoint;
  if ( !this->TransformPositionEventToPoint2D( positionEvent, planarFigureGeometry, point2D ) )
  {
    return false;
  }

  // TODO: check segment of polyline we clicked in
  int nextIndex = -1;

  // We only need to check which position to insert the control point
  // when interacting with a PlanarPolygon. For all other types
  // new control points will always be appended

  /*
  * Added check for "initiallyplaced" due to bug 13097:
  *
  * There are two possible cases in which a point can be inserted into a PlanarPolygon:
  *
  * 1. The figure is currently drawn -> the point will be appended at the end of the figure
  * 2. A point is inserted at a userdefined position after the initial placement of the figure is finished
  *
  * In the second case we need to determine the proper insertion index. In the first case the index always has
  * to be -1 so that the point is appended to the end.
  *
  * These changes are necessary because of a mac os x specific issue: If a users draws a PlanarPolygon then the
  * next point to be added moves according to the mouse position. If then the user left clicks in order to add
  * a point one would assume the last move position is identical to the left click position. This is actually the
  * case for windows and linux but somehow NOT for mac. Because of the insertion logic of a new point in the
  * PlanarFigure then for mac the wrong current selected point is determined.
  *
  * With this check here this problem can be avoided. However a redesign of the insertion logic should be considered
  */
//   bool isFigureFinished = false;
//   planarFigure->GetPropertyList()->GetBoolProperty( "initiallyplaced", isFigureFinished );
  // Replaced with IsFinalized

  mitk::BaseRenderer *renderer = interactionEvent->GetSender();
  const PlaneGeometry *projectionPlane = renderer->GetCurrentWorldPlaneGeometry();

  if ( dynamic_cast<mitk::PlanarPolygon*>( planarFigure ) && planarFigure->IsFinalized())
  {
    nextIndex = this->IsPositionOverFigure(
      positionEvent,
      planarFigure,
      planarFigureGeometry,
      projectionPlane,
      renderer->GetDisplayGeometry(),
      projectedPoint
      );
  }


  // Add point as new control point
  renderer->GetDisplayGeometry()->DisplayToWorld( projectedPoint, projectedPoint );

  if ( planarFigure->IsPreviewControlPointVisible() )
  {
    point2D = planarFigure->GetPreviewControlPoint();
  }

//  int prevPointIndex = planarFigure->GetControlPointForPolylinePoint( nextIndex, 0 );
  if (m_PointMoved) {
      m_PointMoved = false;

      if (m_UndoEnabled)
      {
          int prevPointIndex = planarFigure->GetSelectedControlPoint();
          std::unique_ptr<mitk::PlanarFigureOperation> doOp(new mitk::PlanarFigureOperation(
              OpMOVE, planarFigure->GetControlPoint(prevPointIndex), prevPointIndex));

          std::unique_ptr<mitk::PlanarFigureOperation> undoOp(new mitk::PlanarFigureOperation(
              OpMOVE, m_StartMovePosition, prevPointIndex));

          OperationEvent *operationEvent = new OperationEvent(planarFigure, doOp.release(), undoOp.release(), "Move point");
          m_UndoController->SetOperationEvent(operationEvent);
      } 
  }

  if (planarFigure->IsFinalized()) {
      mitk::OperationEvent::IncCurrGroupEventId();
      mitk::OperationEvent::IncCurrObjectEventId();
      mitk::OperationEvent::ExecuteIncrement();
  }

  if (nextIndex == -1) {
      nextIndex = planarFigure->GetNumberOfControlPoints();
  }
  std::unique_ptr<mitk::PlanarFigureOperation> doOp(new mitk::PlanarFigureOperation(
      OpINSERT, point2D, nextIndex));

  planarFigure->ExecuteOperation(doOp.get());

  if (m_UndoEnabled)
  {
      std::unique_ptr<mitk::PlanarFigureOperation> undoOp(new mitk::PlanarFigureOperation(
          OpREMOVE, point2D, nextIndex));

      OperationEvent *operationEvent = new OperationEvent(planarFigure, doOp.release(), undoOp.release(), "Add point");
      m_UndoController->SetOperationEvent(operationEvent);
  } 


  if ( planarFigure->IsPreviewControlPointVisible() )
  {
    planarFigure->SelectControlPoint( nextIndex );
    planarFigure->ResetPreviewContolPoint();
  }

  // Update rendered scene
  renderer->GetRenderingManager()->RequestUpdateAll();

  return true;
}


bool mitk::PlanarFigureInteractor::AddInitialPoint(StateMachineAction*, InteractionEvent* interactionEvent)
{
    mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  if ( positionEvent == nullptr )
    return false;

  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>( GetDataNode()->GetData() );
  mitk::BaseRenderer *renderer = interactionEvent->GetSender();
  mitk::PlaneGeometry *planarFigureGeometry = dynamic_cast< PlaneGeometry * >( planarFigure->GetGeometry( 0 ) );
  mitk::AbstractTransformGeometry *abstractTransformGeometry = dynamic_cast< AbstractTransformGeometry * >( planarFigure->GetGeometry( 0 ) );

  // Invoke event to notify listeners that placement of this PF starts now
  planarFigure->InvokeEvent( StartPlacementPlanarFigureEvent() );

  // Use PlaneGeometry of the renderer clicked on for this PlanarFigure
  mitk::PlaneGeometry *planeGeometry = const_cast< mitk::PlaneGeometry * >(
    dynamic_cast< const mitk::PlaneGeometry * >(
    renderer->GetSliceNavigationController()->GetCurrentPlaneGeometry() ) );
  if ( planeGeometry != nullptr && abstractTransformGeometry == nullptr)
  {
    planarFigureGeometry = planeGeometry;
    planarFigure->SetPlaneGeometry( planeGeometry );
  }
  else
  {
    return false;
  }

  // Extract point in 2D world coordinates (relative to PlaneGeometry of
  // PlanarFigure)
  Point2D point2D;
  if ( !this->TransformPositionEventToPoint2D( positionEvent, planarFigureGeometry, point2D ) )
  {
    return false;
  }

  mitk::OperationEvent::IncCurrGroupEventId();
  mitk::OperationEvent::IncCurrObjectEventId();
  mitk::OperationEvent::ExecuteIncrement();

  std::unique_ptr<mitk::PlanarFigureOperation> doOp(new mitk::PlanarFigureOperation(
      OpADD, point2D, -1));

  planarFigure->ExecuteOperation(doOp.get());

  if (m_UndoEnabled)
  {
      std::unique_ptr<mitk::PlanarFigureOperation> undoOp(new mitk::PlanarFigureOperation(
          OpUNDOADD, point2D, -1));

      OperationEvent *operationEvent = new OperationEvent(planarFigure, doOp.release(), undoOp.release(), "Add initial point");
      m_UndoController->SetOperationEvent(operationEvent);
  }

  m_StartMovePosition = point2D;
  m_PointMoved = false;

  // Set a bool property indicating that the figure has been placed in
  // the current RenderWindow. This is required so that the same render
  // window can be re-aligned to the PlaneGeometry of the PlanarFigure later
  // on in an application.
  GetDataNode()->SetBoolProperty( "PlanarFigureInitializedWindow", true, renderer );

  // Update rendered scene
  renderer->GetRenderingManager()->RequestUpdateAll();

  return true;
}

bool mitk::PlanarFigureInteractor::StartHovering( StateMachineAction*, InteractionEvent* interactionEvent )
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  if ( positionEvent == nullptr )
    return false;

  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>( GetDataNode()->GetData() );
  mitk::BaseRenderer *renderer = interactionEvent->GetSender();

  if ( !m_IsHovering )
  {
    // Invoke hover event once when the mouse is entering the figure area
    m_IsHovering = true;
    planarFigure->InvokeEvent( StartHoverPlanarFigureEvent() );

    // Set bool property to indicate that planar figure is currently in "hovering" mode
    GetDataNode()->SetBoolProperty( "planarfigure.ishovering", true );

    renderer->GetRenderingManager()->RequestUpdateAll();
  }

  return true;
}

bool mitk::PlanarFigureInteractor::SetPreviewPointPosition( StateMachineAction*, InteractionEvent* interactionEvent )
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  if ( positionEvent == nullptr )
    return false;

  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>( GetDataNode()->GetData() );
  mitk::BaseRenderer *renderer = interactionEvent->GetSender();

  planarFigure->DeselectControlPoint();

  mitk::Point2D pointProjectedOntoLine = positionEvent->GetPointerPositionOnScreen();

  bool selected(false);
  bool isExtendable(false);
  bool isEditable(true);
  GetDataNode()->GetBoolProperty("selected", selected);
  GetDataNode()->GetBoolProperty("planarfigure.isextendable", isExtendable);
  GetDataNode()->GetBoolProperty("planarfigure.iseditable", isEditable );

  if ( selected &&  isExtendable && isEditable )
  {
    renderer->GetDisplayGeometry()->DisplayToWorld( pointProjectedOntoLine, pointProjectedOntoLine );
    planarFigure->SetPreviewControlPoint( pointProjectedOntoLine );
  }

  renderer->GetRenderingManager()->RequestUpdateAll();

  return true;
}

bool mitk::PlanarFigureInteractor::HideControlPoints( StateMachineAction*, InteractionEvent* /*interactionEvent*/ )
{
  GetDataNode()->SetBoolProperty( "planarfigure.drawcontrolpoints", false );
  return true;
}

bool mitk::PlanarFigureInteractor::HidePreviewPoint( StateMachineAction*, InteractionEvent* interactionEvent )
{
  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>( GetDataNode()->GetData() );
  planarFigure->ResetPreviewContolPoint();

  mitk::BaseRenderer *renderer = interactionEvent->GetSender();
  renderer->GetRenderingManager()->RequestUpdateAll();

  return true;
}


bool mitk::PlanarFigureInteractor::CheckFigureHovering( const InteractionEvent* interactionEvent )
{
  const mitk::InteractionPositionEvent* positionEvent = dynamic_cast<const mitk::InteractionPositionEvent*>( interactionEvent );
  if ( positionEvent == nullptr )
    return false;

  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>( GetDataNode()->GetData() );
  mitk::BaseRenderer *renderer = interactionEvent->GetSender();
  const mitk::PlaneGeometry *planarFigureGeometry = planarFigure->GetPlaneGeometry();
  mitk::AbstractTransformGeometry *abstractTransformGeometry = dynamic_cast< AbstractTransformGeometry * >( planarFigure->GetGeometry( 0 ) );
  const PlaneGeometry *projectionPlane = renderer->GetCurrentWorldPlaneGeometry();

  if (renderer->GetMapperID() != mitk::BaseRenderer::Standard2D) {
      return false;
  }

  if ( abstractTransformGeometry != nullptr )
    return false;

  mitk::Point2D pointProjectedOntoLine;
  int previousControlPoint = this->IsPositionOverFigure( positionEvent,
                                                         planarFigure,
                                                         planarFigureGeometry,
                                                         projectionPlane,
                                                         renderer->GetDisplayGeometry(),
                                                         pointProjectedOntoLine
                                                        );

  bool isHovering = (previousControlPoint != -1);

  if ( isHovering )
  {
    return true;
  }
  else
  {
    return false;
  }

  return false;
}


bool mitk::PlanarFigureInteractor::CheckControlPointHovering( const InteractionEvent* interactionEvent )
{
  const mitk::InteractionPositionEvent* positionEvent = dynamic_cast<const mitk::InteractionPositionEvent*>( interactionEvent );
  if ( positionEvent == nullptr )
    return false;

  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>( GetDataNode()->GetData() );
  mitk::BaseRenderer *renderer = interactionEvent->GetSender();
  mitk::PlaneGeometry *planarFigureGeometry = dynamic_cast< PlaneGeometry * >( planarFigure->GetGeometry( 0 ) );
  mitk::AbstractTransformGeometry *abstractTransformGeometry = dynamic_cast< AbstractTransformGeometry * >( planarFigure->GetGeometry( 0 ) );
  const PlaneGeometry *projectionPlane = renderer->GetCurrentWorldPlaneGeometry();

  if (renderer->GetMapperID() != mitk::BaseRenderer::Standard2D) {
      return false;
  }
  if (abstractTransformGeometry != nullptr)
    return false;


  int pointIndex = -1;
  pointIndex = mitk::PlanarFigureInteractor::IsPositionInsideMarker(
    positionEvent,
    planarFigure,
    planarFigureGeometry,
    projectionPlane,
    renderer->GetDisplayGeometry() );

  if ( pointIndex >= 0 )
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool mitk::PlanarFigureInteractor::CheckSelection( const InteractionEvent* /*interactionEvent*/ )
{
  bool selected = false;
  GetDataNode()->GetBoolProperty("selected", selected);

  return selected;
}

bool mitk::PlanarFigureInteractor::SelectFigure( StateMachineAction*, InteractionEvent* /*interactionEvent*/ )
{
  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>( GetDataNode()->GetData() );
  planarFigure->InvokeEvent( SelectPlanarFigureEvent() );
  return false;
}

bool mitk::PlanarFigureInteractor::SelectPoint( StateMachineAction*, InteractionEvent* interactionEvent )
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  if ( positionEvent == nullptr )
    return false;

  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>( GetDataNode()->GetData() );
  mitk::BaseRenderer *renderer = interactionEvent->GetSender();
  mitk::PlaneGeometry *planarFigureGeometry = dynamic_cast< PlaneGeometry * >( planarFigure->GetGeometry( 0 ) );
  mitk::AbstractTransformGeometry *abstractTransformGeometry = dynamic_cast< AbstractTransformGeometry * >( planarFigure->GetGeometry( 0 ) );
  const PlaneGeometry *projectionPlane = renderer->GetCurrentWorldPlaneGeometry();

  if (abstractTransformGeometry != nullptr)
    return false;

  int pointIndex = -1;
  pointIndex = mitk::PlanarFigureInteractor::IsPositionInsideMarker(
    positionEvent,
    planarFigure,
    planarFigureGeometry,
    projectionPlane,
    renderer->GetDisplayGeometry() );

  int prevPointIndex = planarFigure->GetSelectedControlPoint();
  if (prevPointIndex != -1 && m_PointMoved) {
      // We need this due to place-by-drag
      m_PointMoved = false;
      if (m_UndoEnabled)
      {
          std::unique_ptr<mitk::PlanarFigureOperation> doOp(new mitk::PlanarFigureOperation(
              OpMOVE, planarFigure->GetControlPoint(prevPointIndex), prevPointIndex));

          std::unique_ptr<mitk::PlanarFigureOperation> undoOp(new mitk::PlanarFigureOperation(
              OpMOVE, m_StartMovePosition, prevPointIndex));

          OperationEvent *operationEvent = new OperationEvent(planarFigure, doOp.release(), undoOp.release(), "Move point");
          m_UndoController->SetOperationEvent(operationEvent);
      }
  }

  if ( pointIndex >= 0 )
  {
    // If mouse is above control point, mark it as selected
    planarFigure->SelectControlPoint( pointIndex );
    m_StartMovePosition = planarFigure->GetControlPoint(pointIndex);
    m_PointMoved = false;
  }
  else
  {
    planarFigure->DeselectControlPoint();
  }

  return false;
}


bool mitk::PlanarFigureInteractor::CheckPointValidity( const InteractionEvent* interactionEvent )
{
  // Check if the distance of the current point to the previously set point in display coordinates
  // is sufficient (if a previous point exists)

  // Extract display position
  const mitk::InteractionPositionEvent* positionEvent = dynamic_cast<const mitk::InteractionPositionEvent*>( interactionEvent );
  if ( positionEvent == nullptr )
    return false;

  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>( GetDataNode()->GetData() );

  m_LastPointWasValid = IsMousePositionAcceptableAsNewControlPoint( positionEvent, planarFigure );
  return m_LastPointWasValid;
}



bool mitk::PlanarFigureInteractor::RemoveSelectedPoint(StateMachineAction*, InteractionEvent* interactionEvent)
{
  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>( GetDataNode()->GetData() );
  mitk::BaseRenderer *renderer = interactionEvent->GetSender();

  int selectedControlPoint = planarFigure->GetSelectedControlPoint();
  Point2D point2D = planarFigure->GetControlPoint(selectedControlPoint);

  mitk::OperationEvent::IncCurrGroupEventId();
  mitk::OperationEvent::IncCurrObjectEventId();
  mitk::OperationEvent::ExecuteIncrement();
  std::unique_ptr<mitk::PlanarFigureOperation> doOp(new mitk::PlanarFigureOperation(
      OpREMOVE, point2D, selectedControlPoint));

  planarFigure->ExecuteOperation(doOp.get());

  if (m_UndoEnabled)
  {
      std::unique_ptr<mitk::PlanarFigureOperation> undoOp(new mitk::PlanarFigureOperation(
          OpINSERT, point2D, selectedControlPoint));

      OperationEvent *operationEvent = new OperationEvent(planarFigure, doOp.release(), undoOp.release(), "Remove point");
      m_UndoController->SetOperationEvent(operationEvent);
  }


  GetDataNode()->SetBoolProperty( "planarfigure.drawcontrolpoints", true );
  planarFigure->InvokeEvent( EndInteractionPlanarFigureEvent() );
  renderer->GetRenderingManager()->RequestUpdateAll();

  HandleEvent( mitk::InternalEvent::New( renderer, this, "Dummy-Event" ), GetDataNode() );

  return true;
}


bool mitk::PlanarFigureInteractor::RequestContextMenu(StateMachineAction*, InteractionEvent* /*interactionEvent*/)
{
  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>( GetDataNode()->GetData() );

  bool selected = false;
  GetDataNode()->GetBoolProperty("selected", selected);

  // no need to invoke this if the figure is already selected
  if ( !selected )
  {
    planarFigure->InvokeEvent( SelectPlanarFigureEvent() );
  }

  planarFigure->InvokeEvent( ContextMenuPlanarFigureEvent() );

  return true;
}


bool mitk::PlanarFigureInteractor::CheckResetOnPointSelect( const InteractionEvent* /*interactionEvent*/ )
{
  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>( GetDataNode()->GetData() );

  // Reset the PlanarFigure if required
  return planarFigure->ResetOnPointSelect();
}


bool mitk::PlanarFigureInteractor::CheckFigureOnRenderingGeometry( const InteractionEvent* interactionEvent )
{
  const mitk::InteractionPositionEvent* posEvent = dynamic_cast<const mitk::InteractionPositionEvent*>(interactionEvent);

  if ( posEvent == nullptr )
    return false;

  mitk::Point3D worldPoint3D = posEvent->GetPositionInWorld();
  mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>(
    GetDataNode()->GetData() );

  mitk::PlaneGeometry *planarFigurePlaneGeometry = dynamic_cast< PlaneGeometry * >( planarFigure->GetGeometry( 0 ) );
  mitk::AbstractTransformGeometry *abstractTransformGeometry = dynamic_cast< AbstractTransformGeometry * >( planarFigure->GetGeometry( 0 ) );

  if ( abstractTransformGeometry != nullptr)
    return false;

  double planeThickness = planarFigurePlaneGeometry->GetExtentInMM( 2 );
  if ( planarFigurePlaneGeometry->Distance( worldPoint3D ) > planeThickness )
  {
    // don't react, when interaction is too far away
    return false;
  }

  BaseRenderer* renderer = posEvent->GetSender();
  assert(renderer);
  if (renderer->GetMapperID() != mitk::BaseRenderer::Standard2D) {
      // Don't react in 3D
      return false;
  }


  return true;
}

void mitk::PlanarFigureInteractor::SetPrecision( mitk::ScalarType precision )
{
  m_Precision = precision;
}


void mitk::PlanarFigureInteractor::SetMinimumPointDistance( ScalarType minimumDistance )
{
  m_MinimumPointDistance = minimumDistance;
}


bool mitk::PlanarFigureInteractor::TransformPositionEventToPoint2D( const InteractionPositionEvent *positionEvent,
                                                                    const PlaneGeometry *planarFigureGeometry,
                                                                    Point2D &point2D )
{
  mitk::Point3D worldPoint3D = positionEvent->GetPositionInWorld();

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
  const mitk::PlaneGeometry *objectGeometry,
  const mitk::PlaneGeometry *rendererGeometry,
  const mitk::DisplayGeometry *displayGeometry ) const
{
  mitk::Point3D point3D;

  // Map circle point from local 2D geometry into 3D world space
  objectGeometry->Map( point2D, point3D );

  double planeThickness = objectGeometry->GetExtentInMM( 2 );

  // TODO: proper handling of distance tolerance
  if ( rendererGeometry->Distance( point3D ) < planeThickness / 3.0 )
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

  float dist1 = crossPoint.SquaredEuclideanDistanceTo(point);
  float dist2 = endPoint.SquaredEuclideanDistanceTo(point);
  float dist3 = startPoint.SquaredEuclideanDistanceTo(point);

  // Point is inside encompassing rectangle IF
  // - its distance to its projected point is small enough
  // - it is not further outside of the line than the defined tolerance
  if (((dist1 < 20.0) && (l1 > 0.0) && (l2 > 0.0))
      || dist2 < 20.0
      || dist3 < 20.0)
  {
    return true;
  }

  return false;
}


int mitk::PlanarFigureInteractor::IsPositionOverFigure(
  const InteractionPositionEvent *positionEvent,
  PlanarFigure *planarFigure,
  const PlaneGeometry *planarFigureGeometry,
  const PlaneGeometry *rendererGeometry,
  const DisplayGeometry *displayGeometry,
  Point2D& pointProjectedOntoLine ) const
{
  mitk::Point2D displayPosition = positionEvent->GetPointerPositionOnScreen();

  // Iterate over all polylines of planar figure, and check if
  // any one is close to the current display position
  typedef mitk::PlanarFigure::PolyLineType VertexContainerType;

  Point2D polyLinePoint;
  Point2D firstPolyLinePoint;
  Point2D previousPolyLinePoint;
  for ( unsigned short loop=0; loop<planarFigure->GetPolyLinesSize(); ++loop )
  {
    const VertexContainerType polyLine = planarFigure->GetPolyLine( loop );

    bool firstPoint( true );
    for ( VertexContainerType::const_iterator it = polyLine.begin(); it != polyLine.end(); ++it )
    {
      // Get plane coordinates of this point of polyline (if possible)
      if ( !this->TransformObjectToDisplay( *it,
                                            polyLinePoint,
                                            planarFigureGeometry,
                                            rendererGeometry,
                                            displayGeometry )
                                           )
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
        //return std::distance(polyLine.begin(), it);
          mitk::PlanarFigure::PolyLineSegmentInfoType segmentInfo = planarFigure->GetPolyLineSegmentInfo(loop);
          return std::distance(segmentInfo.begin(), std::lower_bound(segmentInfo.begin(), segmentInfo.end(), std::distance(polyLine.begin(), it)));
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
  const InteractionPositionEvent* positionEvent,
  const PlanarFigure *planarFigure,
  const PlaneGeometry *planarFigureGeometry,
  const PlaneGeometry *rendererGeometry,
  const DisplayGeometry *displayGeometry ) const
{
  mitk::Point2D displayPosition = positionEvent->GetPointerPositionOnScreen();

  // Iterate over all control points of planar figure, and check if
  // any one is close to the current display position
  mitk::Point2D displayControlPoint;

  int numberOfControlPoints = planarFigure->GetNumberOfControlPoints();
  for ( int i=0; i<numberOfControlPoints; i++ )
  {
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


  if (dynamic_cast<const mitk::PlanarCircle*>(planarFigure)) {
      mitk::Point2D dummy;
      if (IsPositionOverFigure(positionEvent, const_cast<mitk::PlanarFigure*>(planarFigure), planarFigureGeometry, rendererGeometry, displayGeometry, dummy) != -1) {
          return 1;
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

bool
mitk::PlanarFigureInteractor::IsMousePositionAcceptableAsNewControlPoint(
    const mitk::InteractionPositionEvent* positionEvent,
    const PlanarFigure* planarFigure )
{
  assert(positionEvent && planarFigure);

  BaseRenderer* renderer = positionEvent->GetSender();
  assert(renderer);

  if (renderer->GetMapperID() != mitk::BaseRenderer::Standard2D) {
      return false;
  }

  // Get the timestep to support 3D+t
  int timeStep( renderer->GetTimeStep( planarFigure ) );

  bool tooClose(false);

  const PlaneGeometry *renderingPlane = renderer->GetCurrentWorldPlaneGeometry();

  mitk::PlaneGeometry *planarFigureGeometry =
    dynamic_cast< mitk::PlaneGeometry * >( planarFigure->GetGeometry( timeStep ) );
  mitk::AbstractTransformGeometry *abstractTransformGeometry =
    dynamic_cast< mitk::AbstractTransformGeometry * >( planarFigure->GetGeometry( timeStep ) );

  if ( abstractTransformGeometry != nullptr )
    return false;

  Point2D point2D, correctedPoint;
  // Get the point2D from the positionEvent
  if ( !this->TransformPositionEventToPoint2D( positionEvent, planarFigureGeometry, point2D ) )
  {
    return false;
  }

  // apply the controlPoint constraints of the planarFigure to get the
  // coordinates that would actually be used.
  correctedPoint = const_cast<PlanarFigure*>( planarFigure )->ApplyControlPointConstraints( 0, point2D );

  // map the 2D coordinates of the new point to world-coordinates
  // and transform those to display-coordinates
  mitk::Point3D newPoint3D;
  planarFigureGeometry->Map( correctedPoint, newPoint3D );
  mitk::Point2D newDisplayPosition;
  renderingPlane->Map( newPoint3D, newDisplayPosition );
  renderer->GetDisplayGeometry()->WorldToDisplay( newDisplayPosition, newDisplayPosition );

  for( int i=0; i < (int)planarFigure->GetNumberOfControlPoints(); i++ )
  {
    if ( i != planarFigure->GetSelectedControlPoint() )
    {
      // Try to convert previous point to current display coordinates
      mitk::Point3D previousPoint3D;
      // map the 2D coordinates of the control-point to world-coordinates
      planarFigureGeometry->Map( planarFigure->GetControlPoint( i ), previousPoint3D );

      if ( renderingPlane->Distance( previousPoint3D ) < 0.1 ) // ugly, but assert makes this work
      {
        mitk::Point2D previousDisplayPosition;
        // transform the world-coordinates into display-coordinates
        renderingPlane->Map( previousPoint3D, previousDisplayPosition );
        renderer->GetDisplayGeometry()->WorldToDisplay( previousDisplayPosition, previousDisplayPosition );

        //Calculate the distance. We use display-coordinates here to make
        // the check independent of the zoom-level of the rendering scene.
        double a = newDisplayPosition[0] - previousDisplayPosition[0];
        double b = newDisplayPosition[1] - previousDisplayPosition[1];

        // If point is to close, do not set a new point
        tooClose = (a * a + b * b < m_MinimumPointDistance );
      }
      if ( tooClose )
        return false; // abort loop early
    }
  }

  return !tooClose; // default
}

void mitk::PlanarFigureInteractor::ConfigurationChanged()
{
  mitk::PropertyList::Pointer properties = GetAttributes();

  std::string precision = "";
  if (properties->GetStringProperty("precision", precision))
  {
    m_Precision = atof(precision.c_str());
  }
  else
  {
    m_Precision = (ScalarType) 6.5;
  }

  std::string minPointDistance = "";
  if (properties->GetStringProperty("minPointDistance", minPointDistance))
  {
    m_MinimumPointDistance = atof(minPointDistance.c_str());
  }
  else
  {
    m_MinimumPointDistance = (ScalarType) 25.0;
  }
}
