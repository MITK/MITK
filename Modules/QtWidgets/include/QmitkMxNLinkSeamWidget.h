/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNLinkSeamWidget_h
#define QmitkMxNLinkSeamWidget_h

#include <MitkQtWidgetsExports.h>

#include <QmitkMxNSyncDimension.h>
#include <QmitkRenderWindowProximity.h>

#include <QPointer>
#include <QWidget>

#include <string>
#include <vector>

class QmitkMxNMultiWidget;
class QSplitterHandle;

/**
 * \brief Renders and edits the synchronization relationship of two
 *        within-splitter adjacent MxN cells on their shared border.
 *
 * The seam is the one place in the layout that *is* the pair, so pairwise
 * coupling is shown there: at idle, one short hue segment per group the two
 * cells share (an unlinked pair shows nothing at all); on pointer proximity
 * the segment expands into a chip pill with one chip per dimension plus a
 * navigation-bundle chip and an "open layout editor" hook.
 *
 * Chip states and clicks (per dimension, for this pair):
 *   - filled (both cells share a group): click unlinks the second cell;
 *   - hollow, hinted (exactly one cell linked): click joins the other cell
 *     to that group;
 *   - hollow, neutral (neither linked): click creates a fresh pair group
 *     and links both;
 *   - split (the cells link different groups): shown but disabled - mixed
 *     pairs are the layout editor's job.
 *
 * The widget is parented on the multi widget (a splitter handle is too
 * narrow to host the pill) and follows the handle's geometry via an event
 * filter, inflated perpendicular to the seam so the pill has room. It is
 * mouse-transparent except over the pill while revealed, so splitter
 * dragging and VTK interaction are never shadowed. Non-adjacent and
 * cross-splitter couplings have no seam; the layout editor covers them.
 */
class MITKQTWIDGETS_EXPORT QmitkMxNLinkSeamWidget : public QWidget
{
  Q_OBJECT

public:

  /**
   * \brief Attach a seam to the splitter handle between two cells.
   *
   * \param editor     The owning multi widget; also the Qt parent and the
   *                   engine the chips drive. Must not be null.
   * \param handle     The splitter handle the two cells share. Must not be
   *                   null; the seam follows its geometry and dies with the
   *                   editor's seam rebuild.
   * \param firstId    Cell on the left/top of the handle.
   * \param secondId   Cell on the right/bottom of the handle.
   * \param proximity  The editor-level proximity controller driving the
   *                   reveal. Must not be null and must outlive the seam.
   *
   * \throws mitk::Exception on a null argument.
   */
  QmitkMxNLinkSeamWidget(QmitkMxNMultiWidget* editor,
                         QSplitterHandle* handle,
                         const QString& firstId,
                         const QString& secondId,
                         QmitkRenderWindowProximity* proximity);

  ~QmitkMxNLinkSeamWidget() override;

  /** \brief How a dimension chip presents the pair's coupling. */
  enum class PairState
  {
    None,      // neither cell linked
    OneSided,  // exactly one cell linked
    Shared,    // both cells linked to the same group
    Mixed      // cells linked to different groups (editor-only territory)
  };

  PairState GetDimensionState(QmitkMxNSyncDimension dimension) const;

  /**
   * \brief Apply the chip semantics for one dimension: None -> fresh pair
   *        group for both cells; OneSided -> the unlinked cell joins the
   *        linked cell's group; Shared -> the second cell leaves; Mixed ->
   *        no-op (layout editor territory).
   */
  void ToggleDimension(QmitkMxNSyncDimension dimension);

  /**
   * \brief Bundle chip: when all four navigation dimensions are Shared,
   *        unlink the second cell from all of them; otherwise couple the
   *        pair on all four (joining an existing navigation group of either
   *        cell, or a fresh pair group).
   */
  void ToggleNavigationBundle();

  /** \brief Groups both cells share on at least one dimension (hue order). */
  std::vector<std::string> GetSharedGroups() const;

protected:

  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  bool eventFilter(QObject* watched, QEvent* event) override;

  /** \brief Serves per-chip tooltips while revealed. */
  bool event(QEvent* event) override;

private:

  struct Chip
  {
    QRect rect;
    QmitkMxNSyncDimension dimension;
    bool isBundle = false;
    bool isEditorHook = false;
  };

  /** \brief Recompute this widget's geometry from the handle's. */
  void FollowHandle();

  bool IsRevealed() const;

  /** \brief The pill rectangle in this widget's coordinates. */
  QRect PillRect() const;

  /** \brief Chip layout inside the pill, recomputed on demand. */
  std::vector<Chip> ChipLayout() const;

  void OnProximityStateChanged(QmitkRenderWindowProximity::RegionId id,
                               QmitkRenderWindowProximity::State state);

  void UpdateInteractivity();

  /** \brief The group both cells (or one of them) link on the dimension. */
  std::optional<std::string> LinkedGroup(const QString& windowId,
                                         QmitkMxNSyncDimension dimension) const;

  /** \brief Create a fresh engine group for coupling this pair. */
  std::string MakePairGroup();

  QPointer<QmitkMxNMultiWidget> m_Editor;
  QPointer<QSplitterHandle> m_Handle;
  QString m_FirstId;
  QString m_SecondId;
  QPointer<QmitkRenderWindowProximity> m_Proximity;

  QmitkRenderWindowProximity::RegionId m_Region = -1;
  QmitkRenderWindowProximity::State m_State = QmitkRenderWindowProximity::State::Idle;

};

#endif
