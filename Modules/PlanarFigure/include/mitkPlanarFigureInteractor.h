/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFigureInteractor_h
#define mitkPlanarFigureInteractor_h

#include <MitkPlanarFigureExports.h>

#include <mitkCommon.h>
#include <mitkDataInteractor.h>
#include <mitkNumericTypes.h>

#ifdef __GNUC__
#pragma GCC visibility push(default)
#endif
#include <itkEventObject.h>
#ifdef __GNUC__
#pragma GCC visibility pop
#endif

namespace mitk
{
  class DataNode;
  class PlaneGeometry;
  class PlanarFigure;
  class PositionEvent;
  class BaseRenderer;
  class InteractionPositionEvent;
  class StateMachineAction;

#ifdef __GNUC__
#pragma GCC visibility push(default)
#endif

  // Define events for PlanarFigure interaction notifications
  itkEventMacroDeclaration(PlanarFigureEvent, itk::AnyEvent);
  itkEventMacroDeclaration(StartPlacementPlanarFigureEvent, PlanarFigureEvent);
  itkEventMacroDeclaration(EndPlacementPlanarFigureEvent, PlanarFigureEvent);
  itkEventMacroDeclaration(SelectPlanarFigureEvent, PlanarFigureEvent);
  itkEventMacroDeclaration(StartInteractionPlanarFigureEvent, PlanarFigureEvent);
  itkEventMacroDeclaration(EndInteractionPlanarFigureEvent, PlanarFigureEvent);
  itkEventMacroDeclaration(StartHoverPlanarFigureEvent, PlanarFigureEvent);
  itkEventMacroDeclaration(EndHoverPlanarFigureEvent, PlanarFigureEvent);
  itkEventMacroDeclaration(ContextMenuPlanarFigureEvent, PlanarFigureEvent);
  itkEventMacroDeclaration(PointMovedPlanarFigureEvent, PlanarFigureEvent);

#ifdef __GNUC__
#pragma GCC visibility pop
#endif

  /**
   * \brief Interactor for creating, editing, and manipulating PlanarFigure objects.
   *
   * Handles mouse-based interaction with PlanarFigure objects, including:
   * - Placement of new planar figures by clicking control points
   * - Selection and hovering of existing figures
   * - Moving control points to modify figure geometry
   * - Adding and removing control points
   * - Context menu requests
   *
   * Emits various events (StartPlacementPlanarFigureEvent, EndPlacementPlanarFigureEvent,
   * SelectPlanarFigureEvent, etc.) that other components can observe.
   *
   * The interaction behavior is defined by a state machine configuration loaded
   * during construction.
   *
   * \ingroup MitkPlanarFigureModule
   * \sa PlanarFigure, PlanarFigureMapper2D, DataInteractor
   */
  class MITKPLANARFIGURE_EXPORT PlanarFigureInteractor : public DataInteractor
  {
  public:
    mitkClassMacro(PlanarFigureInteractor, DataInteractor);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Sets the precision for picking control points and figure edges.
     *
     * Defines how close (in display coordinates) a mouse click must be
     * to a control point or figure edge to select it.
     *
     * \param[in] precision The precision value in display units.
     */
    void SetPrecision(ScalarType precision);

    /**
     * \brief Sets the minimal distance between two control points.
     *
     * Prevents adding a new control point too close to an existing one.
     *
     * \param[in] minimumDistance The minimum allowed distance between control points.
     */
    void SetMinimumPointDistance(ScalarType minimumDistance);

    /**
     * \brief Enables continuous points mode.
     *
     * In this mode, control points are added continuously while moving the
     * mouse, rather than requiring individual clicks.
     */
    void EnableContinuousPointsMode();

  protected:
    PlanarFigureInteractor();
    ~PlanarFigureInteractor() override;

    void ConnectActionsAndFunctions() override;

    ////////  Conditions ////////
    bool CheckFigurePlaced(const InteractionEvent *interactionEvent);

    bool CheckFigureHovering(const InteractionEvent *interactionEvent);

    bool CheckControlPointHovering(const InteractionEvent *interactionEvent);

    bool CheckSelection(const InteractionEvent *interactionEvent);

    bool CheckPointValidity(const InteractionEvent *interactionEvent);

    bool CheckFigureFinished(const InteractionEvent *interactionEvent);

    bool CheckResetOnPointSelect(const InteractionEvent *interactionEvent);

    bool CheckFigureOnRenderingGeometry(const InteractionEvent *interactionEvent);

    bool CheckMinimalFigureFinished(const InteractionEvent *interactionEvent);

    bool CheckFigureIsExtendable(const InteractionEvent *interactionEvent);

    bool CheckFigureIsDeletable(const InteractionEvent *interactionEvent);

    bool CheckFigureIsEditable(const InteractionEvent *interactionEvent);

    bool CheckContinuousPointsMode(const InteractionEvent* interactionEvent);

    ////////  Actions ////////

    void FinalizeFigure(StateMachineAction *, InteractionEvent *interactionEvent);

    void MoveCurrentPoint(StateMachineAction *, InteractionEvent *interactionEvent);

    void DeselectPoint(StateMachineAction *, InteractionEvent *interactionEvent);

    void AddPoint(StateMachineAction *, InteractionEvent *interactionEvent);

    void AddInitialPoint(StateMachineAction *, InteractionEvent *interactionEvent);

    void StartHovering(StateMachineAction *, InteractionEvent *interactionEvent);

    void EndHovering(StateMachineAction *, InteractionEvent *interactionEvent);

    void DeleteFigure(StateMachineAction *, InteractionEvent *interactionEvent);

    void PerformPointResetOnSelect(StateMachineAction *, InteractionEvent *interactionEvent);

    void SetPreviewPointPosition(StateMachineAction *, InteractionEvent *interactionEvent);

    void HidePreviewPoint(StateMachineAction *, InteractionEvent *interactionEvent);

    void HideControlPoints(StateMachineAction *, InteractionEvent *interactionEvent);

    void RemoveSelectedPoint(StateMachineAction *, InteractionEvent *interactionEvent);

    void RequestContextMenu(StateMachineAction *, InteractionEvent *interactionEvent);

    void SelectFigure(StateMachineAction *, InteractionEvent *interactionEvent);

    void SelectPoint(StateMachineAction *, InteractionEvent *interactionEvent);

    void EndInteraction(StateMachineAction *, InteractionEvent *interactionEvent);

    bool FilterEvents(InteractionEvent *interactionEvent, DataNode *) override;

    /**
      \brief Used when clicking to determine if a point is too close to the previous point.
      */
    bool IsMousePositionAcceptableAsNewControlPoint(const mitk::InteractionPositionEvent *positionEvent,
                                                    const PlanarFigure *);

    bool TransformPositionEventToPoint2D(const InteractionPositionEvent *positionEvent,
                                         const PlaneGeometry *planarFigureGeometry,
                                         Point2D &point2D);

    bool TransformObjectToDisplay(const mitk::Point2D &point2D,
                                  mitk::Point2D &displayPoint,
                                  const mitk::PlaneGeometry *objectGeometry,
                                  const mitk::PlaneGeometry *rendererGeometry,
                                  const mitk::BaseRenderer *renderer) const;

    /** \brief Returns true if the first specified point is in proximity of the line defined
     * the other two point; false otherwise.
     *
     * Proximity is defined as the rectangle around the line with pre-defined distance
     * from the line. */
    bool IsPointNearLine(const mitk::Point2D &point,
                         const mitk::Point2D &startPoint,
                         const mitk::Point2D &endPoint,
                         mitk::Point2D &projectedPoint) const;

    /** \brief Returns true if the point contained in the passed event (in display coordinates)
     * is over the planar figure (with a pre-defined tolerance range); false otherwise. */
    int IsPositionOverFigure(const InteractionPositionEvent *positionEvent,
                             PlanarFigure *planarFigure,
                             const PlaneGeometry *planarFigureGeometry,
                             const PlaneGeometry *rendererGeometry,
                             Point2D &pointProjectedOntoLine) const;

    /** \brief Returns the index of the marker (control point) over which the point contained
     * in the passed event (in display coordinates) currently is; -1 if the point is not over
     * a marker. */
    int IsPositionInsideMarker(const InteractionPositionEvent *positionEvent,
                               const PlanarFigure *planarFigure,
                               const PlaneGeometry *planarFigureGeometry,
                               const PlaneGeometry *rendererGeometry,
                               const BaseRenderer *renderer) const;

    void LogPrintPlanarFigureQuantities(const PlanarFigure *planarFigure);

    void ConfigurationChanged() override;

  private:
    /** \brief to store the value of precision to pick a point */
    ScalarType m_Precision;

    /** \brief Store the minimal distance between two control points. */
    ScalarType m_MinimumPointDistance;

    /** \brief True if the mouse is currently hovering over the image. */
    bool m_IsHovering;

    bool m_ContinuousPointsMode = false;
  };
}

#endif
