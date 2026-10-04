/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNGroupJoinMode.h"

#include <QAction>
#include <QCoreApplication>
#include <QMenu>
#include <QMimeData>

const char* const QmitkMxNCellsMimeType = "application/x-mitk-mxn-cells";
const char* const QmitkMxNGroupMimeType = "application/x-mitk-mxn-group";
const char* const QmitkMxNAskModeMimeType = "application/x-mitk-mxn-askmode";

QMimeData* QmitkMxNCreateCellsMimeData(const QStringList& windowIds, bool askMode)
{
  auto* mimeData = new QMimeData();
  mimeData->setData(QmitkMxNCellsMimeType, windowIds.join(QStringLiteral("\n")).toUtf8());
  if (askMode)
  {
    mimeData->setData(QmitkMxNAskModeMimeType, QByteArray());
  }
  return mimeData;
}

QmitkMxNGroupJoinMode QmitkMxNJoinModeFromModifiers(Qt::KeyboardModifiers modifiers)
{
  // Alt merges (overwriting collisions), Shift fills only empty axes; a plain
  // drop replaces. Ctrl is deliberately not used - cell selection already binds
  // it to multi-select, so it must keep its selection meaning during a drag.
  if (modifiers.testFlag(Qt::AltModifier))
  {
    return QmitkMxNGroupJoinMode::MergeOverwriteCollisions;
  }
  if (modifiers.testFlag(Qt::ShiftModifier))
  {
    return QmitkMxNGroupJoinMode::FillEmpty;
  }
  return QmitkMxNGroupJoinMode::Replace;
}

std::vector<QmitkMxNJoinModeEntry> QmitkMxNJoinModeMenuEntries()
{
  const auto tr = [](const char* text) { return QCoreApplication::translate("QmitkMxNGroupJoinMode", text); };
  return { { QmitkMxNGroupJoinMode::Replace, tr("Replace the cells' synchronization") },
           { QmitkMxNGroupJoinMode::MergeOverwriteCollisions, tr("Merge, overwriting collisions") },
           { QmitkMxNGroupJoinMode::FillEmpty, tr("Fill only unsynchronized axes") } };
}

std::optional<QmitkMxNGroupJoinMode> QmitkMxNResolveJoinMode(const QMimeData* mimeData,
                                                             Qt::KeyboardModifiers modifiers,
                                                             const QPoint& globalPosition)
{
  if (nullptr == mimeData || !mimeData->hasFormat(QmitkMxNAskModeMimeType))
  {
    return QmitkMxNJoinModeFromModifiers(modifiers);
  }

  QMenu menu;
  std::vector<QAction*> actions;
  const auto entries = QmitkMxNJoinModeMenuEntries();
  actions.reserve(entries.size());
  for (const auto& entry : entries)
  {
    actions.push_back(menu.addAction(entry.label));
  }
  menu.addSeparator();
  menu.addAction(QCoreApplication::translate("QmitkMxNGroupJoinMode", "Cancel"));

  const QAction* chosen = menu.exec(globalPosition);
  for (std::size_t i = 0; i < actions.size(); ++i)
  {
    if (chosen == actions[i])
    {
      return entries[i].mode;
    }
  }
  return std::nullopt;
}
