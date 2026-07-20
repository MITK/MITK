/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRenderWindowProximity_h
#define QmitkRenderWindowProximity_h

#include <MitkQtWidgetsExports.h>

#include <QObject>
#include <QPoint>
#include <QPointer>
#include <QRect>

#include <functional>
#include <map>

class QTimer;
class QWidget;

/**
 * \brief Owns the idle -> hint -> active reveal state machine for one MxN
 *        cell's viewport furniture.
 *
 * Surfaces register rectangular hot regions (in the cell widget's coordinate
 * space, queried lazily so they stay correct across resizes); the controller
 * tracks the pointer across the whole cell and emits a state per region:
 *
 *   - Idle:   the pointer is outside the cell (or the controller is
 *             suppressed, see SetSuppressed).
 *   - Hint:   the pointer is inside the cell but not near the region.
 *   - Active: the pointer is within ActivationDistance of the region
 *             rectangle (with a hysteresis band once active).
 *
 * Transitions to a higher state are immediate; transitions to a lower state
 * are delayed by CollapseDelayMs and cancelled if the pointer returns, so
 * furniture never flaps at a threshold. A region only rises to Active on a
 * button-free hover: while a mouse button is held the escalation is withheld,
 * so the frame never reveals mid-gesture (drawing, crosshairing or windowing
 * over the image); a region that is already Active is left untouched, so a
 * drag begun on the furniture itself keeps it up. One instance per cell keeps timing
 * and hysteresis defined in exactly one place; the constants are public so
 * surfaces animate consistently (RevealDurationMs is the reveal easing
 * budget for the painting layer, the controller itself switches states
 * without animation).
 *
 * Pointer tracking is fed by event filters on the cell frame and on the
 * widgets registered via AddEventSource. Qt delivers mouse moves to the
 * child under the cursor, never to the enclosing frame, so the cell filter
 * alone would be blind over the image; the owning editor therefore registers
 * the widgets that cover the hot regions (the render window, and any
 * furniture widgets of its own) as additional sources. Enter/leave events do
 * propagate along the parent chain, so the cell frame still provides
 * reliable inside/outside tracking (the Hint state) over unregistered
 * children such as the utility row.
 */
class MITKQTWIDGETS_EXPORT QmitkRenderWindowProximity : public QObject
{
  Q_OBJECT

public:

  enum class State
  {
    Idle = 0,
    Hint = 1,
    Active = 2
  };
  Q_ENUM(State)

  using RegionId = int;

  static constexpr int RevealDurationMs = 140;
  static constexpr int CollapseDelayMs = 300;
  static constexpr int ActivationDistance = 58;
  static constexpr int HysteresisBand = 16;

  /**
   * \brief Track the pointer over 'cell' and evaluate registered regions
   *        against it.
   *
   * \param cell    The cell widget whose coordinate space regions live in
   *                (for MxN, the enclosing QmitkRenderWindowWidget QFrame).
   *                Must not be null. Mouse tracking is enabled on it.
   * \param parent  Optional QObject parent.
   *
   * \throws mitk::Exception if 'cell' is null.
   */
  explicit QmitkRenderWindowProximity(QWidget* cell, QObject* parent = nullptr);

  ~QmitkRenderWindowProximity() override;

  /**
   * \brief Register an additional widget whose pointer events feed this
   *        controller (mouse tracking is enabled on it).
   *
   * Continuous positions are only delivered to the widget under the cursor,
   * so every widget that covers a hot region needs to be a source; the cell
   * frame itself only contributes enter/leave (and its own uncovered
   * margins). Sources must live in the same window as the cell (their events
   * are resolved against the cell via global coordinates).
   *
   * \throws mitk::Exception if 'source' is null.
   */
  void AddEventSource(QWidget* source);

  /**
   * \brief Register a furniture hot region.
   *
   * \param regionInCellCoords  Callback returning the region rectangle in
   *                            the cell's coordinate space. Queried lazily
   *                            on every evaluation, so the region follows
   *                            layout changes without re-registration. An
   *                            invalid/empty rectangle is allowed; such a
   *                            region never reaches Active but still gets
   *                            Hint while the pointer is in the cell.
   *                            Must not be null.
   * \param activationDistance  Pointer-to-rectangle distance (px) below
   *                            which the region goes Active. Surfaces with
   *                            a tighter reveal (e.g. seams between cells)
   *                            pass a smaller value; timing and hysteresis
   *                            stay uniform. Must be positive.
   *
   * \return  Id used in the StateChanged signal and the query/unregister
   *          calls. The region starts in the state matching the current
   *          pointer; a change away from Idle is emitted immediately.
   *
   * \throws mitk::Exception if the callback is null or the distance is not
   *         positive.
   */
  RegionId RegisterRegion(std::function<QRect()> regionInCellCoords,
                          int activationDistance = ActivationDistance);

  /**
   * \brief Remove a region; no further StateChanged is emitted for its id.
   *
   * \throws mitk::Exception on an unknown id.
   */
  void UnregisterRegion(RegionId id);

  /**
   * \brief The region's current state.
   *
   * \throws mitk::Exception on an unknown id.
   */
  State GetRegionState(RegionId id) const;

  /**
   * \brief Clean-view suppression: while suppressed, every region reports
   *        Idle regardless of the pointer (the drop to Idle is immediate,
   *        not collapse-delayed). Unsuppressing re-evaluates against the
   *        last known pointer position immediately.
   */
  void SetSuppressed(bool suppressed);
  bool IsSuppressed() const;

  /**
   * \brief Feed a pointer position in the cell's coordinate space.
   *
   * \param positionInCell  Pointer position in the cell's coordinates.
   * \param buttonsPressed  Whether any mouse button is currently held. A
   *                        region escalates to Active only on a button-free
   *                        hover; while a button is down the escalation is
   *                        withheld (the frame must not reveal mid-gesture),
   *                        though an already-Active region is left as is.
   *
   * Normally driven by the installed event filter; public so tests and
   * custom event sources can feed synthetic positions headlessly.
   */
  void HandlePointerMoved(const QPoint& positionInCell, bool buttonsPressed = false);

  /** \brief Feed "the pointer left the cell". See HandlePointerMoved. */
  void HandlePointerLeft();

Q_SIGNALS:

  void StateChanged(RegionId id, State state);

protected:

  bool eventFilter(QObject* watched, QEvent* event) override;

private:

  struct Region
  {
    std::function<QRect()> rectQuery;
    int activationDistance = ActivationDistance;
    State state = State::Idle;
    QTimer* collapseTimer = nullptr;
  };

  /** \brief The state the region should be in for the current pointer. */
  State ComputeState(const Region& region) const;

  /**
   * \brief Apply upward changes immediately; arm the collapse timer for
   *        downward ones (cancelled when a later evaluation goes up again).
   */
  void EvaluateRegion(RegionId id, Region& region);

  void EvaluateAllRegions();

  void ApplyState(RegionId id, Region& region, State state);

  void OnCollapseTimeout(RegionId id);

  QPointer<QWidget> m_Cell;
  std::map<RegionId, Region> m_Regions;
  RegionId m_NextRegionId = 0;

  QPoint m_PointerPosition;
  bool m_PointerInside = false;
  bool m_ButtonsPressed = false;
  bool m_Suppressed = false;

};

#endif
