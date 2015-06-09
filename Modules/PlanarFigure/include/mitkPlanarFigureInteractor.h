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


#ifndef MITKPLANARFIGUREINTERACTOR_H_HEADER_INCLUDED
#define MITKPLANARFIGUREINTERACTOR_H_HEADER_INCLUDED

#include <MitkPlanarFigureExports.h>

#include "mitkCommon.h"
#include "mitkNumericTypes.h"
#include "mitkDataInteractor.h"

#pragma GCC visibility push(default)
#include <itkEventObject.h>
#pragma GCC visibility pop

namespace mitk
{

class DataNode;
class PlaneGeometry;
class DisplayGeometry;
class PlanarFigure;
class PositionEvent;
class BaseRenderer;
class InteractionPositionEvent;
class StateMachineAction;

#pragma GCC visibility push(default)

// Define events for PlanarFigure interaction notifications
itkEventMacro( PlanarFigureEvent, itk::AnyEvent );
itkEventMacro( StartPlacementPlanarFigureEvent, PlanarFigureEvent );
itkEventMacro( EndPlacementPlanarFigureEvent, PlanarFigureEvent );
itkEventMacro( SelectPlanarFigureEvent, PlanarFigureEvent );
itkEventMacro( StartInteractionPlanarFigureEvent, PlanarFigureEvent );
itkEventMacro( EndInteractionPlanarFigureEvent, PlanarFigureEvent );
itkEventMacro( StartHoverPlanarFigureEvent, PlanarFigureEvent );
itkEventMacro( EndHoverPlanarFigureEvent, PlanarFigureEvent );
itkEventMacro( ContextMenuPlanarFigureEvent, PlanarFigureEvent );

#pragma GCC visibility pop


/**
  * \brief Interaction with mitk::PlanarFigure objects via control-points
  *
  * @ingroup MitkPlanarFigureModule
  */
class MITKPLANARFIGURE_EXPORT PlanarFigureInteractor : public DataInteractor
{
public:
  mitkClassMacro(PlanarFigureInteractor, DataInteractor);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** \brief Sets the amount of precision */
  void SetPrecision( ScalarType precision );

  /** \brief Sets the minimal distance between two control points. */
  void SetMinimumPointDistance( ScalarType minimumDistance );

protected:

  PlanarFigureInteractor();
  virtual ~PlanarFigureInteractor();

  virtual void ConnectActionsAndFunctions() override;

  ////////  Conditions ////////
  bool CheckFigurePlaced( const InteractionEvent* interactionEvent );

  bool CheckFigureHovering( const InteractionEvent* interactionEvent );

  bool CheckControlPointHovering( const InteractionEvent* interactionEvent );

  bool CheckSelection( const InteractionEvent* interactionEvent );

  bool CheckPointValidity( const InteractionEvent* interactionEvent );

  bool CheckFigureFinished( const InteractionEvent* interactionEvent );

  bool CheckResetOnPointSelect( const InteractionEvent* interactionEvent );

  bool CheckFigureOnRenderingGeometry( const InteractionEvent* interactionEvent );

  bool CheckMinimalFigureFinished( const InteractionEvent* interactionEvent );

  bool CheckFigureIsExtendable( const InteractionEvent* interactionEvent );


  ////////  Actions ////////

  bool FinalizeFigure( StateMachineAction*, InteractionEvent* interactionEvent );

  bool MoveCurrentPoint(StateMachineAction*, InteractionEvent* interactionEvent);

  bool DeselectPoint(StateMachineAction*, InteractionEvent* interactionEvent);

  bool AddPoint(StateMachineAction*, InteractionEvent* interactionEvent);

  bool AddInitialPoint(StateMachineAction*, InteractionEvent* interactionEvent);

  bool StartHovering( StateMachineAction*, InteractionEvent* interactionEvent );

  bool EndHovering( StateMachineAction*, InteractionEvent* interactionEvent );

  bool DeleteFigure( StateMachineAction*, InteractionEvent* interactionEvent );

  bool SetPreviewPointPosition( StateMachineAction*, InteractionEvent* interactionEvent );

  bool HidePreviewPoint( StateMachineAction*, InteractionEvent* interactionEvent );

  bool HideControlPoints( StateMachineAction*, InteractionEvent* interactionEvent );

  bool RemoveSelectedPoint(StateMachineAction*, InteractionEvent* interactionEvent);

  bool RequestContextMenu(StateMachineAction*, InteractionEvent* interactionEvent);

  bool SelectFigure( StateMachineAction*, InteractionEvent* interactionEvent );

  bool SelectPoint( StateMachineAction*, InteractionEvent* interactionEvent );

  bool EndInteraction( StateMachineAction*, InteractionEvent* interactionEvent );



  /**
    \brief Used when clicking to determine if a point is too close to the previous point.
    */
  bool IsMousePositionAcceptableAsNewControlPoint( const mitk::InteractionPositionEvent* positionEvent, const PlanarFigure* );

  bool TransformPositionEventToPoint2D( const InteractionPositionEvent* positionEvent,
                                        const PlaneGeometry *planarFigureGeometry,
                                        Point2D &point2D );

  bool TransformObjectToDisplay( const mitk::Point2D &point2D,
    mitk::Point2D &displayPoint,
    const mitk::PlaneGeometry *objectGeometry,
    const mitk::PlaneGeometry *rendererGeometry,
    const mitk::DisplayGeometry *displayGeometry ) const;

  /** \brief Returns true if the first specified point is in proximity of the line defined
   * the other two point; false otherwise.
   *
   * Proximity is defined as the rectangle around the line with pre-defined distance
   * from the line. */
  bool IsPointNearLine( const mitk::Point2D& point,
    const mitk::Point2D& startPoint, const mitk::Point2D& endPoint, mitk::Point2D& projectedPoint ) const;

  /** \brief Returns true if the point contained in the passed event (in display coordinates)
   * is over the planar figure (with a pre-defined tolerance range); false otherwise. */
  int IsPositionOverFigure(
    const InteractionPositionEvent* positionEvent,
    PlanarFigure *planarFigure,
    const PlaneGeometry *planarFigureGeometry,
    const PlaneGeometry *rendererGeometry,
    const DisplayGeometry *displayGeometry,
    Point2D& pointProjectedOntoLine) const;

  /** \brief Returns the index of the marker (control point) over which the point contained
   * in the passed event (in display coordinates) currently is; -1 if the point is not over
   * a marker. */
  int IsPositionInsideMarker(
    const InteractionPositionEvent* positionEvent,
    const PlanarFigure *planarFigure,
    const PlaneGeometry *planarFigureGeometry,
    const PlaneGeometry *rendererGeometry,
    const DisplayGeometry *displayGeometry ) const;

  void LogPrintPlanarFigureQuantities( const PlanarFigure *planarFigure );

  virtual void ConfigurationChanged() override;

private:

  /** \brief to store the value of precision to pick a point */
  ScalarType m_Precision;

  /** \brief Store the minimal distance between two control points. */
  ScalarType m_MinimumPointDistance;

  /** \brief True if the mouse is currently hovering over the image. */
  bool m_IsHovering;

  bool m_LastPointWasValid;

  //mitk::PlanarFigure::Pointer m_PlanarFigure;
};

}

#endif // MITKPLANARFIGUREINTERACTOR_H_HEADER_INCLUDED
