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

#include <mitkDataInteractor.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkNumericTypes.h>

#ifdef __GNUC__
#pragma GCC visibility push(default)
#endif
#include <itkEventObject.h>
#ifdef __GNUC__
#pragma GCC visibility pop
#endif

#include <vtkRenderWindow.h>
#include <vtkWeakPointer.h>

#include <optional>

namespace mitk
{
  class BaseRenderer;
  class PlanarFigure;
  class StateMachineAction;

#ifdef __GNUC__
#pragma GCC visibility push(default)
#endif

  // Define events for PlanarFigure interaction notifications
  itkEventMacroDeclaration(PlanarFigureEvent, itk::AnyEvent);
  itkEventMacroDeclaration(EndPlacementPlanarFigureEvent, PlanarFigureEvent);
  itkEventMacroDeclaration(SelectPlanarFigureEvent, PlanarFigureEvent);
  itkEventMacroDeclaration(EndInteractionPlanarFigureEvent, PlanarFigureEvent);
  itkEventMacroDeclaration(PointMovedPlanarFigureEvent, PlanarFigureEvent);

#ifdef __GNUC__
#pragma GCC visibility pop
#endif

  /**
   * \brief Places and edits planar figures in 2D render windows.
   *
   * A figure without control points is placed by the first press: a span
   * from the press to the release, or to the next click, for figures placed
   * by a span; one point per click for the others, finished by a double
   * click or Enter, where Backspace or Delete removes the last point. A
   * figure whose shape is locked follows the mouse until it is dropped by a
   * click; beside the image it is hidden, and a click does not drop it.
   *
   * Placed figures show their handles while hovered. Dragging a handle moves
   * it, dragging the stroke or the inside moves the whole figure. Pressing a
   * figure selects it. Centers and vertices stay within the bounds of the
   * plane, extents of round figures may leave them. Figures that can insert
   * control points take a new one by Ctrl+click on their stroke, which can be
   * dragged right away, and lose one by Ctrl+click on it.
   *
   * The cursor of the render window shows what a press would do: a pointing
   * hand on handles, a move cursor on strokes and insides, and a crosshair
   * where Ctrl+click would insert or remove a point.
   *
   * Placed figures ignore input while an armed tool holds the
   * ExclusiveInteraction, e.g. so that a new figure can be started inside
   * them. Where visible figures overlap, the inside of a figure yields to
   * figures hit at their handles or strokes, and to figures with a smaller
   * inside, and a stroke yields to figures hit at their handles.
   *
   * The figure is notified by EndPlacementPlanarFigureEvent,
   * SelectPlanarFigureEvent (only when a press hits a figure whose node is
   * not selected yet, so a selection of several nodes is kept),
   * PointMovedPlanarFigureEvent and EndInteractionPlanarFigureEvent. Mappers
   * read what to highlight from GetFeedback().
   *
   * Configuration parameters, in logical pixels: \c hitTolerance (4) around
   * handles and strokes, \c dragThreshold (3) a press has to move to drag.
   *
   * \sa PlanarFigure, PlanarFigureVtkMapper2D
   */
  class MITKPLANARFIGURE_EXPORT PlanarFigureInteractor : public DataInteractor
  {
  public:
    mitkClassMacro(PlanarFigureInteractor, DataInteractor);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief What a render window shows of the interaction with the figure. */
    struct Feedback
    {
      const BaseRenderer *Renderer = nullptr;
      bool IsHovered = false;
      std::optional<std::size_t> HoveredHandle;

      /** \brief Where a Ctrl+click would insert a control point, in plane coordinates. */
      std::optional<Point2D> InsertionPreview;

      /** \brief Whether a Ctrl+click would remove the hovered handle. */
      bool IsRemovalPreview = false;

      bool operator==(const Feedback &) const = default;
    };

    /** \brief The feedback for a render window; empty for other windows than the one interacted in. */
    Feedback GetFeedback(const BaseRenderer *renderer) const;

  protected:
    PlanarFigureInteractor();
    ~PlanarFigureInteractor() override;

    void ConnectActionsAndFunctions() override;
    void ConfigurationChanged() override;
    void DataNodeChanged() override;
    bool FilterEvents(InteractionEvent *interactionEvent, DataNode *dataNode) override;

  private:
    enum class HitKind
    {
      None,
      Handle,
      Removal,
      Insertion,
      Stroke,
      Interior
    };

    struct Hit
    {
      HitKind Kind = HitKind::None;

      /** \brief The handle hit, also for a removal. */
      std::optional<std::size_t> Handle;

      /** \brief For an insertion, the index of the new control point and its position in plane coordinates. */
      std::size_t InsertionIndex = 0;
      Point2D InsertionPosition = Point2D(0.0);
    };

    PlanarFigure *GetFigure() const;

    /** \brief Position of the event in plane coordinates of the figure, or nothing if the event is not in its plane. */
    std::optional<Point2D> GetPlanePosition(const InteractionEvent *interactionEvent) const;

    /** \brief Where the position hits the figure, regardless of other figures. Held Ctrl turns hits into insertions and removals. */
    Hit HitTest(const InteractionPositionEvent *positionEvent, bool isControlHeld) const;

    /**
     * \brief Where the event hits the figure, or nothing if the hit is left to an armed tool or another figure.
     *
     * Events without a position, like a press of Ctrl, are taken at the position of the last move.
     */
    Hit GetHit(const InteractionEvent *interactionEvent) const;
    double GetMillimetersPerPixel(const BaseRenderer *renderer) const;
    double GetDisplayDistance(const InteractionEvent *interactionEvent, const Point2D &displayPosition) const;

    /**
     * \brief Replaces the feedback and updates the render windows of the previous and the new one.
     *
     * Feedback of another interactor is cleared, since the dispatcher may not
     * have sent it the event that moved the mouse away from its figure.
     */
    void SetFeedback(const Feedback &feedback, vtkRenderWindow *renderWindow);

    bool IsEmpty(const InteractionEvent *interactionEvent);
    bool IsOnFlatPlane(const InteractionEvent *interactionEvent);
    bool IsPlacedBySpan(const InteractionEvent *interactionEvent);
    bool IsStamp(const InteractionEvent *interactionEvent);
    bool IsInsideSlice(const InteractionEvent *interactionEvent);
    bool IsComplete(const InteractionEvent *interactionEvent);
    bool IsHit(const InteractionEvent *interactionEvent);
    bool IsBeyondDragThreshold(const InteractionEvent *interactionEvent);
    bool IsValidPoint(const InteractionEvent *interactionEvent);
    bool ReachesMaximum(const InteractionEvent *interactionEvent);
    bool CanFinish(const InteractionEvent *interactionEvent);
    bool IsRemovable(const InteractionEvent *interactionEvent);
    bool IsInsertable(const InteractionEvent *interactionEvent);

    void UpdateHover(StateMachineAction *, InteractionEvent *interactionEvent);
    void EndHover(StateMachineAction *, InteractionEvent *interactionEvent);
    void Press(StateMachineAction *, InteractionEvent *interactionEvent);
    void Drag(StateMachineAction *, InteractionEvent *interactionEvent);
    void EndDrag(StateMachineAction *, InteractionEvent *interactionEvent);
    void RemovePoint(StateMachineAction *, InteractionEvent *interactionEvent);
    void InsertPoint(StateMachineAction *, InteractionEvent *interactionEvent);
    void BeginPlacement(StateMachineAction *, InteractionEvent *interactionEvent);
    void MoveFloatingPoint(StateMachineAction *, InteractionEvent *interactionEvent);
    void AppendPoint(StateMachineAction *, InteractionEvent *interactionEvent);
    void RemoveFloatingPoint(StateMachineAction *, InteractionEvent *interactionEvent);
    void RemoveLastPoint(StateMachineAction *, InteractionEvent *interactionEvent);
    void EndPlacement(StateMachineAction *, InteractionEvent *interactionEvent);
    void MoveStamp(StateMachineAction *, InteractionEvent *interactionEvent);
    void HideStamp(StateMachineAction *, InteractionEvent *interactionEvent);

    /** \brief Shows the stamp in the given window only, or nowhere for nullptr. */
    void ShowStampIn(const BaseRenderer *renderer);

    // In logical pixels, also for configurations that leave them out or give invalid values.
    static constexpr double DefaultHitTolerance = 4.0;
    static constexpr double DefaultDragThreshold = 3.0;

    double m_HitTolerance = DefaultHitTolerance;
    double m_DragThreshold = DefaultDragThreshold;

    Feedback m_Feedback;

    vtkWeakPointer<vtkRenderWindow> m_FeedbackWindow;

    /** \brief Where the last press happened, in display coordinates. */
    Point2D m_PressPosition;

    /** \brief The last event with a position while hovering, for events without one. */
    InteractionPositionEvent::ConstPointer m_LastMove;

    /**
     * \brief What the last press grabbed, and the offset of the grabbed handle,
     * or of the first control point for the whole figure, from the press in
     * plane coordinates.
     */
    Hit m_Grab;
    Vector2D m_GrabOffset;

    /** \brief Where the span of the figure being placed starts, in plane coordinates. */
    Point2D m_SpanStart;
  };
}

#endif
