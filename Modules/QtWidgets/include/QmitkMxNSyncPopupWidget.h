/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNSyncPopupWidget_h
#define QmitkMxNSyncPopupWidget_h

#include <MitkQtWidgetsExports.h>

#include <QmitkMxNMultiWidget.h>
#include <QmitkMxNSyncDimension.h>

#include <QWidget>

class QComboBox;
class QDoubleSpinBox;
class QSpinBox;
class QToolButton;

/**
 * \brief Interim per-cell control for the MxN navigation synchronization
 *        links (pan, zoom, slice, crosshair).
 *
 * One row per dimension: an editable group selector (empty = unlinked; typing
 * a new name creates the group on commit), the dimension's offset editor
 * where the dimension carries one, and a re-converge button for the offset
 * dimensions. The widget is a passive view plus request emitter: it never
 * touches the editor's link state itself. The owning multi widget connects
 * the request signals and pushes state back via SetKnownGroups /
 * SetLinkState.
 */
class MITKQTWIDGETS_EXPORT QmitkMxNSyncPopupWidget : public QWidget
{
  Q_OBJECT

public:

  explicit QmitkMxNSyncPopupWidget(QWidget* parent = nullptr);

  /** \brief Replace the group names offered by the dimension's selector
   *         (preserving the current selection). The selector always offers
   *         an explicit "(not linked)" entry first; group names share one
   *         namespace across dimensions, so callers pass the same list to
   *         every dimension. */
  void SetKnownGroups(QmitkMxNSyncDimension dimension, const QStringList& groups);

  /** \brief Display the cell's current link state for the dimension.
   *         An empty group means unlinked. Emits no request signals. */
  void SetLinkState(QmitkMxNSyncDimension dimension,
                    const QString& group,
                    const QmitkMxNMultiWidget::SyncOffset& offset);

  /** \brief Surface a rejected link request (e.g. malformed group name) as
   *         a tooltip at the dimension's selector, so the silent snap-back
   *         to the previous state is explained. */
  void ShowLinkError(QmitkMxNSyncDimension dimension, const QString& message);

Q_SIGNALS:

  /**
   * \brief The user edited the dimension's group or offset. An empty group
   *        requests unlinking; otherwise the offset carries the dimension's
   *        typed value.
   */
  void LinkChangeRequested(QmitkMxNSyncDimension dimension,
                           const QString& group,
                           const QmitkMxNMultiWidget::SyncOffset& offset);

  /** \brief The user requested re-converging the dimension's current group. */
  void ReconvergeRequested(QmitkMxNSyncDimension dimension, const QString& group);

  /**
  * \brief The user requested re-initializing the geometry of this cell's
  *        geometry-authority component (its slice/orientation link
  *        neighborhood).
  */
  void ReinitGeometryRequested();

private:

  struct Row
  {
    QmitkMxNSyncDimension dimension;
    QComboBox* groupSelector = nullptr;
    QSpinBox* sliceOffset = nullptr;        // Slice row only
    QDoubleSpinBox* zoomOffset = nullptr;   // Zoom row only
    QDoubleSpinBox* panOffsetX = nullptr;   // Pan row only
    QDoubleSpinBox* panOffsetY = nullptr;   // Pan row only
    QToolButton* reconvergeButton = nullptr;
  };

  Row* FindRow(QmitkMxNSyncDimension dimension);
  QmitkMxNMultiWidget::SyncOffset CurrentOffset(const Row& row) const;
  void EmitLinkChange(Row& row);

  std::vector<Row> m_Rows;
};

#endif
