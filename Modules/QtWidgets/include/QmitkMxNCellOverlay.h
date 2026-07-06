/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNCellOverlay_h
#define QmitkMxNCellOverlay_h

#include <MitkQtWidgetsExports.h>

#include <QmitkOverlayWidget.h>
#include <QmitkRenderWindowProximity.h>

#include <mitkDataNode.h>
#include <mitkLevelWindow.h>

#include <vtkSmartPointer.h>
#include <vtkType.h>

#include <QImage>
#include <QPointer>
#include <QRect>

class QmitkMxNMultiWidget;
class QmitkRenderWindowWidget;
class vtkRenderWindow;

/**
 * \brief The composite viewport-furniture overlay of one MxN cell.
 *
 * One Qt overlay per cell paints all furniture over the render window and
 * owns its sub-region hit testing (rather than stacking one full-rect
 * overlay per surface, which would multiply z-order and hit-test ambiguity).
 * Currently carried furniture, windowing/LUT:
 *
 *   - a passive LUT ribbon along the right edge (the node's actual colormap,
 *     2 px at idle) that widens on proximity into a draggable control with
 *     bound labels: dragging the body shifts the level, dragging the ends
 *     moves the window bounds, all routed through the editor's synchronized
 *     per-renderer level-window path so group members follow;
 *   - a corner readout `W <window> L <level>` bottom-left, always visible by
 *     default (clinical habit), with a hue dot naming the cell's
 *     windowing/LUT group when linked;
 *   - on proximity: a colormap chip that opens a palette menu (routed through
 *     the editor's per-renderer lookup-table path) and double-click numeric
 *     level/window entry (routed through the by-value setter).
 *
 * The overlay is mouse-transparent while idle. While a furniture region is
 * active it takes input only over the visible furniture bounds (widget
 * mask), so VTK interaction is never shadowed elsewhere; it feeds the
 * proximity controller as an event source, keeping reveal state alive while
 * the pointer is over the furniture itself.
 *
 * Slice/time breadcrumbs share the overlay: a `slice/steps` readout bottom
 * right, a bottom hairline with a position tick (a second, dashed hairline
 * appears only for time-resolved data), and, on proximity to the bottom
 * edge, the standard slice-navigation slider hosted over the image. The top
 * edge shows a thin strip for the auto-hidden utility toolbar and reveals it
 * on approach. A right-click (without drag - the zoom/windowing gestures
 * stay untouched) opens the cell's context menu: per-renderer data
 * visibility, view direction, and the cell's sync actions.
 *
 * Values refresh on the cell's VTK render-end events, coalesced to the next
 * event-loop cycle - not on a timer.
 *
 * This is a Qt layer on top of the VTK scene: the passive ribbon and readout
 * are deliberately not VTK annotations, keeping one code path for the
 * passive and active layers.
 */
class MITKQTWIDGETS_EXPORT QmitkMxNCellOverlay : public QmitkOverlayWidget
{
  Q_OBJECT

public:

  /**
   * \brief Attach the overlay to a cell.
   *
   * \param cell       The cell to decorate; also the Qt parent (the overlay
   *                   dies with it). Must not be null and must already hold
   *                   its render window.
   * \param editor     The owning editor, used for the synchronized
   *                   level-window / lookup-table writes and group lookups.
   *                   Must not be null and must outlive the cell.
   * \param proximity  The cell's proximity controller; the overlay registers
   *                   its furniture regions and follows the emitted states.
   *                   Must not be null and must outlive the overlay.
   *
   * \throws mitk::Exception on a null argument or a cell without a render
   *         window.
   */
  QmitkMxNCellOverlay(QmitkRenderWindowWidget* cell,
                      QmitkMxNMultiWidget* editor,
                      QmitkRenderWindowProximity* proximity);

  ~QmitkMxNCellOverlay() override;

  /**
   * \brief Preference-controlled default visibility of the corner readout.
   *        When off, the readout follows the proximity reveal like all other
   *        furniture instead of being always-on.
   */
  void SetReadoutVisible(bool visible);

  /** \brief Clean-view mode: while set, the overlay paints nothing at all. */
  void SetCleanView(bool cleanView);

protected:

  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;

  /** \brief Adds render-window context-menu handling to the base's
   *         parent-resize tracking. */
  bool eventFilter(QObject* watched, QEvent* event) override;

private:

  enum class DragMode
  {
    None,
    Level,       // ribbon body: shift the level
    UpperBound,  // ribbon top end: move the upper window bound
    LowerBound   // ribbon bottom end: move the lower window bound
  };

  /** \brief The render window's rect in cell (== overlay) coordinates. */
  QRect RenderWindowRect() const;

  /** \brief The ribbon rect at its active (wide) or passive (hairline) width. */
  QRect RibbonRect(bool active) const;

  /** \brief Bounding rect of the corner readout text plus hue dot. */
  QRect ReadoutRect() const;

  /** \brief The colormap chip beside the readout (active layer only). */
  QRect ColormapChipRect() const;

  /** \brief Slice `pos/steps` readout, bottom right (left of the ribbon). */
  QRect SliceReadoutRect() const;

  /** \brief Hot region along the bottom edge revealing the slice slider. */
  QRect BottomStripRect() const;

  /** \brief Hot region along the cell's top edge revealing the toolbar. */
  QRect TopStripRect() const;

  void UpdateSliceSlider();

  void OpenContextMenu(const QPoint& globalPosition);

  bool IsRevealed(QmitkRenderWindowProximity::State state, bool alwaysOn) const;

  void OnProximityStateChanged(QmitkRenderWindowProximity::RegionId id,
                               QmitkRenderWindowProximity::State state);

  /**
   * \brief Sync mouse transparency and the input/paint mask with the current
   *        reveal state: input only over visible furniture while active,
   *        fully transparent otherwise. A running drag pins the interactive
   *        state so a mid-drag collapse cannot break the implicit grab.
   */
  void UpdateInteractivity();

  /** \brief Coalesce VTK render-end notifications into one refresh per cycle. */
  void ScheduleValueRefresh();

  /** \brief Re-read node, level/window, and LUT; repaint on change. */
  void RefreshValues();

  /** \brief Topmost image node visible in this cell's renderer, or null. */
  mitk::DataNode::Pointer ResolveTopImageNode() const;

  void RebuildLutStrip();

  void OpenNumericEntry();
  void OpenColormapMenu();

  /** \brief World value per dragged pixel, from the drag-start window. */
  double DragScale() const;

  static void OnVtkRenderEnd(vtkObject* caller, unsigned long eventId,
                             void* clientData, void* callData);

  QmitkRenderWindowWidget* m_Cell;
  QmitkMxNMultiWidget* m_Editor;
  QPointer<QmitkRenderWindowProximity> m_Proximity;
  vtkSmartPointer<vtkRenderWindow> m_VtkRenderWindow;
  unsigned long m_VtkObserverTag = 0;

  QmitkRenderWindowProximity::RegionId m_RibbonRegion = -1;
  QmitkRenderWindowProximity::RegionId m_ReadoutRegion = -1;
  QmitkRenderWindowProximity::RegionId m_BottomRegion = -1;
  QmitkRenderWindowProximity::RegionId m_TopRegion = -1;
  QmitkRenderWindowProximity::State m_RibbonState = QmitkRenderWindowProximity::State::Idle;
  QmitkRenderWindowProximity::State m_ReadoutState = QmitkRenderWindowProximity::State::Idle;
  QmitkRenderWindowProximity::State m_BottomState = QmitkRenderWindowProximity::State::Idle;
  QmitkRenderWindowProximity::State m_TopState = QmitkRenderWindowProximity::State::Idle;

  bool m_ReadoutVisible = true;
  bool m_CleanView = false;
  bool m_RefreshPending = false;

  mitk::DataNode::Pointer m_TopNode;
  mitk::LevelWindow m_LevelWindow;
  bool m_HasLevelWindow = false;
  QImage m_LutStrip;
  vtkMTimeType m_LutMTime = 0;

  DragMode m_DragMode = DragMode::None;
  QPoint m_LastDragPosition;
  double m_DragScale = 1.0;

  unsigned int m_SlicePosition = 0;
  unsigned int m_SliceSteps = 0;
  unsigned int m_TimePosition = 0;
  unsigned int m_TimeSteps = 0;

  QWidget* m_SliceSlider = nullptr;        // lazily created breadcrumb slider
  QPoint m_RightPressPosition;             // context-menu drag suppression

};

#endif
