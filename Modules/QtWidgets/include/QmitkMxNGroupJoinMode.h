/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNGroupJoinMode_h
#define QmitkMxNGroupJoinMode_h

#include <MitkQtWidgetsExports.h>

#include <QString>
#include <QStringList>
#include <Qt>

#include <optional>
#include <vector>

class QMimeData;
class QPoint;
class QWidget;

/**
 * \brief How a cell's existing links are treated when it joins a group - the
 *        drop-mode selector for assigning windows to a group in the MxN editor.
 *
 * Replace is the default and the effect of a plain drop: the cell wholly joins
 * the target group, so every other link is cleared first (its selection reverts
 * to the default group). FillEmpty sets only the cell's currently-unlinked axes,
 * leaving its existing links untouched. MergeOverwriteCollisions sets the group's
 * axes (overwriting a colliding link) but keeps the cell's links on axes the
 * group does not cover.
 */
enum class QmitkMxNGroupJoinMode
{
  Replace,
  FillEmpty,
  MergeOverwriteCollisions
};

/**
 * \brief Drag payloads between the MxN editor's cells and its group cards.
 *
 * Dragged cells carry the newline-separated window ids; a dragged group carries
 * its id. A drag started with the right button additionally carries the
 * ask-mode marker (empty payload), which makes the drop ask for its join mode
 * through a menu instead of reading modifier keys.
 */
MITKQTWIDGETS_EXPORT extern const char* const QmitkMxNCellsMimeType;
MITKQTWIDGETS_EXPORT extern const char* const QmitkMxNGroupMimeType;
MITKQTWIDGETS_EXPORT extern const char* const QmitkMxNAskModeMimeType;

/** \brief The payload of a cell drag: 'windowIds', with the ask-mode marker
 *         when 'askMode' is set. The caller owns the result (QDrag takes it). */
MITKQTWIDGETS_EXPORT QMimeData* QmitkMxNCreateCellsMimeData(const QStringList& windowIds, bool askMode);

/** \brief One entry of the join-mode menu a right-button drop offers. */
struct QmitkMxNJoinModeEntry
{
  QmitkMxNGroupJoinMode mode;
  QString label;
};

/**
 * \brief The join mode a drop's keyboard modifiers request: Alt =
 *        MergeOverwriteCollisions, Shift = FillEmpty, none = Replace (the
 *        default). Read at drop time (on release), not at drag initiation, so a
 *        modifier held while starting a drag has no effect on the mode. Shared
 *        by every drop target so the modifier meaning is identical everywhere.
 */
MITKQTWIDGETS_EXPORT QmitkMxNGroupJoinMode QmitkMxNJoinModeFromModifiers(Qt::KeyboardModifiers modifiers);

/** \brief The join modes a right-button drop offers, in menu order. The single
 *         source of the offered set, so every drop target presents the same one. */
MITKQTWIDGETS_EXPORT std::vector<QmitkMxNJoinModeEntry> QmitkMxNJoinModeMenuEntries();

/**
 * \brief The join mode a drop asks for: a drag carrying the ask-mode marker
 *        pops the menu at 'globalPosition' and yields the chosen mode, or
 *        nothing when the user dismisses it; any other drag reads its
 *        modifiers. Shared by every drop target so the gesture means the same
 *        everywhere.
 */
MITKQTWIDGETS_EXPORT std::optional<QmitkMxNGroupJoinMode> QmitkMxNResolveJoinMode(const QMimeData* mimeData,
                                                                                Qt::KeyboardModifiers modifiers,
                                                                                QWidget* parent,
                                                                                const QPoint& globalPosition);

#endif
