/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNCellMapWidget_h
#define QmitkMxNCellMapWidget_h

#include <MitkQtWidgetsExports.h>

#include <QmitkMxNGroupJoinMode.h>

#include <QPointer>
#include <QRect>
#include <QStringList>
#include <QWidget>

#include <vector>

class QmitkMxNMultiWidget;

/**
 * \brief Interactive miniature map of an MxN editor's cell layout, colored by
 *        synchronization-group membership.
 *
 * Tiles mirror the live splitter geometry. Each tile shows the cell's name,
 * its navigation group (fill in the group hue when the four navigation
 * dimensions agree; a neutral fill marks hybrid cells), and a per-dimension
 * "sync barcode" along the tile bottom: one slot per dimension in the fixed
 * engine order (pan, zoom, slice, crosshair, orientation, windowing, LUT),
 * filled with the linked group's hue, left as a gap when unsynced, with a
 * notch marking links that carry an offset.
 *
 * Cells are selected by click or Ctrl-click (a press on empty space clears the
 * selection); selected tiles can be dragged onto a group (or a group dropped onto
 * a tile) - the widget only reports these gestures via signals, the owning editor
 * performs the engine mutation.
 *
 * Mime types: dragged cells use "application/x-mitk-mxn-cells" (newline-
 * separated window ids); accepted group drops use
 * "application/x-mitk-mxn-group" (the group id).
 */
class MITKQTWIDGETS_EXPORT QmitkMxNCellMapWidget : public QWidget
{
  Q_OBJECT

public:

  static const char* CellsMimeType;
  static const char* GroupMimeType;

  explicit QmitkMxNCellMapWidget(QWidget* parent = nullptr);
  ~QmitkMxNCellMapWidget() override;

  void SetMultiWidget(QmitkMxNMultiWidget* multiWidget);

  /** \brief Re-read cells, geometry, and link state; keeps the selection
   *         where the cells still exist. */
  void Rebuild();

  QStringList GetSelectedWindowIds() const;

  /** \brief Set the selected tiles (e.g. to mirror the editor's active render
   *         window). Emits SelectionChanged only when the selection changes. */
  void SetSelectedWindowIds(const QStringList& windowIds);

  /** \brief The join mode a drop's keyboard modifiers request: Alt =
   *         MergeOverwriteCollisions, Shift = FillEmpty, none = Replace (the
   *         default). Read at drop time (on release), not at drag initiation, so a
   *         modifier held while starting a drag from a group card has no effect on
   *         the mode. Shared by both drop targets (this map and the editor's group
   *         cards) so the modifier meaning is identical everywhere. */
  static QmitkMxNGroupJoinMode JoinModeFromModifiers(Qt::KeyboardModifiers modifiers);

Q_SIGNALS:

  void SelectionChanged(const QStringList& windowIds);

  /** \brief A group was dropped onto a tile (or the current selection), with the
   *         join mode the drop's modifiers requested. */
  void AssignRequested(const QString& group, const QStringList& windowIds,
                       QmitkMxNGroupJoinMode mode);

protected:

  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void dragLeaveEvent(QDragLeaveEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

  QSize minimumSizeHint() const override;

private:

  struct Tile
  {
    QString windowId;
    QString label;
    QRectF normalizedRect;  // in [0,1]^2 relative to the editor rect
    QRect mapRect;          // scaled into this widget, cached on layout
  };

  void UpdateTileRects();
  int TileAt(const QPoint& position) const;
  void SetSelection(const QStringList& windowIds);
  void StartCellDrag();

  QPointer<QmitkMxNMultiWidget> m_MultiWidget;
  std::vector<Tile> m_Tiles;
  QStringList m_Selection;

  QPoint m_PressPosition;  // press origin for the cell-drag start threshold
  bool m_DragCandidate = false;
  int m_DropTargetTile = -1;  // tile highlighted under a hovering group drag

};

#endif
