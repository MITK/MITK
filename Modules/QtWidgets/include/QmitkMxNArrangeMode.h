/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNArrangeMode_h
#define QmitkMxNArrangeMode_h

#include <MitkQtWidgetsExports.h>

#include <QmitkMxNGroupJoinMode.h>
#include <QmitkMxNSyncDimension.h>

#include <QColor>
#include <QObject>
#include <QPointer>
#include <QStringList>

#include <optional>

class QMimeData;
class QmitkMxNMultiWidget;

/**
 * \brief Arrange mode of one MxN editor: while it is on, every cell's peek
 *        plate is the surface for selecting cells and assigning them to
 *        synchronization groups.
 *
 * Holds only arrangement state - on/off, the cell selection with its range
 * anchor, and the sync highlight - plus the rule for what a drop targets. The
 * engine mutation stays with whoever handles AssignRequested and
 * RemoveRequested (the layout editor). Interaction outside the plates is never affected.
 *
 * Several surfaces drive the highlight (a plate glyph, a cell's utility-strip
 * barcode, the layout editor's cards and matrix). The latest one to set it owns
 * it, and a surface clearing its own highlight never clears another's, so a
 * pointer leaving one surface cannot wipe what the next one just showed.
 */
class MITKQTWIDGETS_EXPORT QmitkMxNArrangeMode : public QObject
{
  Q_OBJECT

public:

  /** \brief The surface a highlight comes from. */
  enum class HighlightSource
  {
    Plate,
    Strip,
    Editor
  };

  /** \brief Created and owned by 'multiWidget', whose cells it arranges. */
  explicit QmitkMxNArrangeMode(QmitkMxNMultiWidget* multiWidget);

  void SetActive(bool active);
  bool IsActive() const;

  QStringList GetSelectedWindowIds() const;

  /** \brief Replace the selection from outside (a mirror of another surface's
   *         selection). Keeps the range anchor. Emits SelectionChanged only
   *         when the selection changes. */
  void SetSelectedWindowIds(const QStringList& windowIds);

  /** \brief Select nothing, as a user action: the range anchor goes too, so
   *         the next Shift-click cannot span from a cell no longer marked. */
  void ClearSelection();

  /**
   * \brief A press on 'windowId''s plate, with the file-explorer selection
   *        model: plain selects just this cell (a press on an already selected
   *        cell holds the selection so a drag can carry all of it, and a click
   *        on the only selected cell deselects it), Ctrl
   *        toggles it, Shift replaces the selection with the geometric range
   *        from the anchor, Ctrl+Shift adds that range. A right press takes an
   *        unselected cell into the selection and arms a drag that asks for its
   *        join mode on drop.
   *
   * Returns whether a drag may start from this press. Shift presses never
   * start one: Shift is also the FillEmpty drop modifier.
   */
  bool PressCell(const QString& windowId, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);

  /** \brief The press ended. Unless it became a drag ('dragged'), a plain
   *         press that held a wider selection collapses it to the pressed cell. */
  void ReleaseCell(bool dragged);

  /** \brief The payload of a drag started from the last press: the selection,
   *         marked to ask for its join mode when the press was a right press. */
  QMimeData* CreateDragMimeData() const;

  /** \brief Drop the selected ids and the anchor whose cells no longer exist. */
  void PruneToExistingCells();

  /** \brief A group was dropped on 'targetWindowId': emits AssignRequested for
   *         the whole selection when the target is selected, else for the
   *         target alone. */
  void RequestAssign(const QString& group, const QString& targetWindowId, QmitkMxNGroupJoinMode mode);

  /** \brief Take 'windowId' out of 'group': emits RemoveRequested for the
   *         whole selection when 'windowId' is selected, else for it alone. */
  void RequestRemove(const QString& group, const QString& windowId);

  /** \brief Emphasise 'axis' on every plate and bump the frames of 'windowIds'
   *         in 'hue'; the list may be empty (emphasis alone). 'source' becomes the
   *         highlight's owner. */
  void SetHighlight(HighlightSource source, std::optional<QmitkMxNSyncAxis> axis,
                    const QStringList& windowIds, const QColor& hue);

  /** \brief Clear the highlight if 'source' owns it; otherwise nothing. */
  void ClearHighlight(HighlightSource source);

  std::optional<QmitkMxNSyncAxis> GetHighlightAxis() const;
  QStringList GetHighlightedWindowIds() const;
  QColor GetHighlightHue() const;

Q_SIGNALS:

  void ActiveChanged(bool active);
  void SelectionChanged(const QStringList& windowIds);
  void HighlightChanged();
  void AssignRequested(const QString& group, const QStringList& windowIds, QmitkMxNGroupJoinMode mode);
  void RemoveRequested(const QString& group, const QStringList& windowIds);

private:

  void SetSelection(const QStringList& windowIds);

  /** \brief Every cell the rectangle spanned by the 'anchor' and 'target'
   *         cells touches. Geometric rather than by row and column, because a
   *         loaded layout need not be a uniform grid. A missing anchor
   *         degenerates to just 'target'. */
  QStringList CellsBetween(const QString& anchor, const QString& target) const;

  QPointer<QmitkMxNMultiWidget> m_MultiWidget;
  bool m_Active = false;

  QStringList m_Selection;
  QString m_Anchor;
  QString m_PressedWindowId;  // a plain press that held a wider selection
  bool m_PressedSoleSelection = false;  // ...on the one cell already selected
  bool m_DragAsksMode = false;

  std::optional<HighlightSource> m_HighlightSource;
  std::optional<QmitkMxNSyncAxis> m_HighlightAxis;
  QStringList m_HighlightedWindowIds;
  QColor m_HighlightHue;
};

#endif
