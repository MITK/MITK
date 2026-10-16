/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkCrashDumpListWidget.h>

#include "QmitkCrashDumpUiUtils.h"

#include <mitkCrashDumpFacility.h>

#include <QDateTime>
#include <QFileInfo>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLocale>
#include <QSignalBlocker>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>

namespace
{
  QString LogLabel(const mitk::CrashDumpInfo& dump)
  {
    if (!dump.SessionLog.empty())
      return "available";

    return dump.RunInfo.has_value() ? "not available" : "unknown";
  }

  QString VersionLabel(const mitk::CrashDumpInfo& dump)
  {
    if (!dump.RunInfo.has_value() || dump.RunInfo->Release.empty())
      return "unknown";

    return QString::fromStdString(dump.RunInfo->Release);
  }
}

QmitkCrashDumpListWidget::QmitkCrashDumpListWidget(QWidget* parent)
  : QWidget(parent),
    m_Tree(new QTreeWidget(this)),
    m_Checkable(false)
{
  m_Tree->setObjectName("crashDumpTree");
  m_Tree->setRootIsDecorated(false);
  m_Tree->setUniformRowHeights(true);
  m_Tree->setItemsExpandable(false);
  m_Tree->setAllColumnsShowFocus(true);
  m_Tree->setTextElideMode(Qt::ElideMiddle);
  m_Tree->setHeaderLabels({ "Kind", "Time", "Size", "Version", "Log", "Status" });
  m_Tree->header()->setStretchLastSection(false);

  this->SetSelectionEnabled(true);
  m_Tree->installEventFilter(this);

  connect(m_Tree, &QTreeWidget::itemSelectionChanged, this, &QmitkCrashDumpListWidget::SelectionChanged);
  connect(m_Tree, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem*, int column) {
    if (column == KindColumn)
      emit CheckedChanged();
  });

  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_Tree);
}

QmitkCrashDumpListWidget::~QmitkCrashDumpListWidget() = default;

void QmitkCrashDumpListWidget::SetDumps(const std::vector<mitk::CrashDumpInfo>& dumps,
  const std::set<std::filesystem::path>& provisionalPaths)
{
  m_Dumps = dumps;

  // Populating changes items; the signals are emitted once at the end.
  const QSignalBlocker blocker(m_Tree);
  m_Tree->clear();

  for (std::size_t i = 0; i < m_Dumps.size(); ++i)
  {
    const auto& dump = m_Dumps[i];
    const auto path = QmitkCrashDumpUi::ToQString(dump.Path);

    QString status = "kept";
    if (provisionalPaths.find(dump.Path) != provisionalPaths.end())
      status = "provisional (this session)";
    else if (dump.Unacknowledged)
      status = "new";

    auto* item = new QTreeWidgetItem(m_Tree);
    item->setData(KindColumn, Qt::UserRole, static_cast<qulonglong>(i));
    item->setText(KindColumn, QmitkCrashDumpUi::KindLabel(dump.Kind));
    item->setText(TimeColumn, QLocale().toString(QFileInfo(path).lastModified(), QLocale::ShortFormat));
    item->setText(SizeColumn, QLocale().formattedDataSize(static_cast<qint64>(dump.SizeInBytes), 2, QLocale::DataSizeTraditionalFormat));
    item->setText(VersionColumn, VersionLabel(dump));
    item->setText(LogColumn, LogLabel(dump));
    item->setText(StatusColumn, status);

    for (int column = 0; column < m_Tree->columnCount(); ++column)
      item->setToolTip(column, path);

    if (!dump.SessionLog.empty())
      item->setToolTip(LogColumn, QmitkCrashDumpUi::ToQString(dump.SessionLog));

    item->setToolTip(VersionColumn, item->text(VersionColumn));
  }

  for (int column = 0; column < m_Tree->columnCount(); ++column)
    m_Tree->resizeColumnToContents(column);

  // Development builds carry long version strings (with commit and local
  // changes); capped, they are elided instead of pushing the columns after
  // them out of view.
  const int maxVersionWidth = m_Tree->fontMetrics().horizontalAdvance("MITK Workbench v2026.06-000-g0000");
  if (m_Tree->columnWidth(VersionColumn) > maxVersionWidth)
    m_Tree->setColumnWidth(VersionColumn, maxVersionWidth);

  this->ApplyCheckable();

  emit SelectionChanged();
  emit CheckedChanged();
}

std::vector<mitk::CrashDumpInfo> QmitkCrashDumpListWidget::GetDumps() const
{
  return m_Dumps;
}

std::vector<mitk::CrashDumpInfo> QmitkCrashDumpListWidget::GetSelectedDumps() const
{
  std::vector<mitk::CrashDumpInfo> selected;

  // Table order, not click order, so that callers see newest first.
  for (int row = 0; row < m_Tree->topLevelItemCount(); ++row)
  {
    const auto* item = m_Tree->topLevelItem(row);
    if (item->isSelected())
      selected.push_back(m_Dumps[item->data(KindColumn, Qt::UserRole).toULongLong()]);
  }

  return selected;
}

void QmitkCrashDumpListWidget::SetCheckable(bool checkable)
{
  m_Checkable = checkable;
  this->UpdateFocusPolicy();

  {
    const QSignalBlocker blocker(m_Tree);
    this->ApplyCheckable();
  }

  emit CheckedChanged();
}

void QmitkCrashDumpListWidget::ApplyCheckable()
{
  for (int row = 0; row < m_Tree->topLevelItemCount(); ++row)
  {
    auto* item = m_Tree->topLevelItem(row);

    if (m_Checkable)
    {
      item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
      item->setCheckState(KindColumn, Qt::Checked);
    }
    else
    {
      item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
      item->setData(KindColumn, Qt::CheckStateRole, QVariant());
    }
  }
}

std::vector<mitk::CrashDumpInfo> QmitkCrashDumpListWidget::GetCheckedDumps() const
{
  std::vector<mitk::CrashDumpInfo> checked;

  if (!m_Checkable)
    return checked;

  for (int row = 0; row < m_Tree->topLevelItemCount(); ++row)
  {
    const auto* item = m_Tree->topLevelItem(row);
    if (item->checkState(KindColumn) == Qt::Checked)
      checked.push_back(m_Dumps[item->data(KindColumn, Qt::UserRole).toULongLong()]);
  }

  return checked;
}

void QmitkCrashDumpListWidget::SetAllChecked(bool checked)
{
  if (!m_Checkable)
    return;

  {
    const QSignalBlocker blocker(m_Tree);

    for (int row = 0; row < m_Tree->topLevelItemCount(); ++row)
      m_Tree->topLevelItem(row)->setCheckState(KindColumn, checked ? Qt::Checked : Qt::Unchecked);
  }

  emit CheckedChanged();
}

QString QmitkCrashDumpListWidget::KindLabel(mitk::DumpKind kind)
{
  return QmitkCrashDumpUi::KindLabel(kind);
}

void QmitkCrashDumpListWidget::SetColumnHidden(Column column, bool hidden)
{
  m_Tree->setColumnHidden(column, hidden);
}

void QmitkCrashDumpListWidget::FitToContents()
{
  const int contentWidth = this->FitColumnsAndHeight();
  m_Tree->setMinimumWidth(contentWidth);
}

void QmitkCrashDumpListWidget::FitHeightToContents()
{
  const int contentWidth = this->FitColumnsAndHeight();

  // Every column but the version keeps its width; the version column, which
  // carries the long strings of development builds, elides instead.
  const int minimumVersionWidth = m_Tree->fontMetrics().horizontalAdvance("MITK Workbench");
  m_Tree->setMinimumWidth(
    std::min(contentWidth, contentWidth - m_Tree->columnWidth(VersionColumn) + minimumVersionWidth));
}

int QmitkCrashDumpListWidget::FitColumnsAndHeight()
{
  // Measured in the font and with the item margins of the style sheet,
  // which a list that has not been shown yet does not have otherwise.
  m_Tree->ensurePolished();

  // Left-aligned columns fitted exactly would nearly touch.
  const int gap = 3 * m_Tree->fontMetrics().averageCharWidth();

  // Also lifts the cap SetDumps() puts on the version column: a list that
  // takes all the width it needs pushes no column out of view.
  for (int column = 0; column < m_Tree->columnCount(); ++column)
  {
    m_Tree->resizeColumnToContents(column);
    m_Tree->setColumnWidth(column, m_Tree->columnWidth(column) + gap);
  }

  auto* header = m_Tree->header();
  const int frame = 2 * m_Tree->frameWidth();
  const int contentWidth = header->length() + frame;

  int height = header->sizeHint().height() + frame;
  for (int row = 0; row < m_Tree->topLevelItemCount(); ++row)
    height += m_Tree->sizeHintForRow(row);

  m_Tree->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_Tree->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_Tree->setFixedHeight(height);
  header->setSectionResizeMode(VersionColumn, QHeaderView::Stretch);
  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

  return contentWidth;
}

void QmitkCrashDumpListWidget::SetSelectionEnabled(bool enabled)
{
  m_Tree->setSelectionMode(enabled ? QAbstractItemView::ExtendedSelection : QAbstractItemView::NoSelection);
  this->UpdateFocusPolicy();
}

void QmitkCrashDumpListWidget::UpdateFocusPolicy()
{
  const bool selectable = m_Tree->selectionMode() != QAbstractItemView::NoSelection;
  m_Tree->setFocusPolicy(selectable || m_Checkable ? Qt::StrongFocus : Qt::NoFocus);
}

bool QmitkCrashDumpListWidget::eventFilter(QObject* watched, QEvent* event)
{
  // Without selection, Qt moves the current row with the arrow keys only on
  // styles that set SH_ItemView_MovementWithoutUpdatingSelection (the macOS
  // style does not), so the navigation keys are handled here on every style.
  if (watched != m_Tree || event->type() != QEvent::KeyPress ||
      m_Tree->selectionMode() != QAbstractItemView::NoSelection)
    return QWidget::eventFilter(watched, event);

  const auto* keyEvent = static_cast<const QKeyEvent*>(event);
  const auto modifiers = keyEvent->modifiers();
  if (modifiers != Qt::NoModifier && modifiers != Qt::KeypadModifier)
    return QWidget::eventFilter(watched, event);

  QTreeWidgetItem* const current = m_Tree->currentItem();
  QTreeWidgetItem* const first = m_Tree->topLevelItem(0);
  QTreeWidgetItem* const last = m_Tree->topLevelItem(m_Tree->topLevelItemCount() - 1);
  QTreeWidgetItem* target = nullptr;

  switch (keyEvent->key())
  {
    case Qt::Key_Up:
      target = current != nullptr ? m_Tree->itemAbove(current) : last;
      break;

    case Qt::Key_Down:
      target = current != nullptr ? m_Tree->itemBelow(current) : first;
      break;

    case Qt::Key_Home:
      target = first;
      break;

    case Qt::Key_End:
      target = last;
      break;

    default:
      return QWidget::eventFilter(watched, event);
  }

  // At either end of the list the key is passed on, so that an enclosing
  // scroll area can still scroll.
  if (target == nullptr || target == current)
    return QWidget::eventFilter(watched, event);

  m_Tree->setCurrentItem(target);
  m_Tree->scrollToItem(target);

  return true;
}
