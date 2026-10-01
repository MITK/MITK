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
#include <QLocale>
#include <QScrollBar>
#include <QTreeWidget>
#include <QVBoxLayout>

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
    m_Tree(new QTreeWidget(this))
{
  m_Tree->setObjectName("crashDumpTree");
  m_Tree->setRootIsDecorated(false);
  m_Tree->setUniformRowHeights(true);
  m_Tree->setItemsExpandable(false);
  m_Tree->setAllColumnsShowFocus(true);
  m_Tree->setHeaderLabels({ "Kind", "Time", "Size", "Version", "Log", "Status" });
  m_Tree->header()->setStretchLastSection(false);

  this->SetSelectionEnabled(true);

  connect(m_Tree, &QTreeWidget::itemSelectionChanged, this, &QmitkCrashDumpListWidget::SelectionChanged);

  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_Tree);
}

QmitkCrashDumpListWidget::~QmitkCrashDumpListWidget() = default;

void QmitkCrashDumpListWidget::SetDumps(const std::vector<mitk::CrashDumpInfo>& dumps,
  const std::set<std::filesystem::path>& provisionalPaths)
{
  m_Dumps = dumps;

  std::set<std::filesystem::path> unacknowledged;
  for (const auto& dump : mitk::CrashDumpFacility::ListUnacknowledgedDumps())
    unacknowledged.insert(dump.Path);

  m_Tree->clear();

  for (std::size_t i = 0; i < m_Dumps.size(); ++i)
  {
    const auto& dump = m_Dumps[i];
    const auto path = QmitkCrashDumpUi::ToQString(dump.Path);

    QString status = "kept";
    if (provisionalPaths.find(dump.Path) != provisionalPaths.end())
      status = "provisional (this session)";
    else if (unacknowledged.find(dump.Path) != unacknowledged.end())
      status = "new";

    auto* item = new QTreeWidgetItem(m_Tree);
    item->setData(KindColumn, Qt::UserRole, static_cast<qulonglong>(i));
    item->setText(KindColumn, QmitkCrashDumpUi::KindLabel(dump.Kind));
    item->setText(TimeColumn, QLocale().toString(QFileInfo(path).lastModified(), QLocale::ShortFormat));
    item->setText(SizeColumn, QLocale().formattedDataSize(static_cast<qint64>(dump.SizeInBytes)));
    item->setText(VersionColumn, VersionLabel(dump));
    item->setText(LogColumn, LogLabel(dump));
    item->setText(StatusColumn, status);

    for (int column = 0; column < m_Tree->columnCount(); ++column)
      item->setToolTip(column, path);

    if (!dump.SessionLog.empty())
      item->setToolTip(LogColumn, QmitkCrashDumpUi::ToQString(dump.SessionLog));
  }

  for (int column = 0; column < m_Tree->columnCount(); ++column)
    m_Tree->resizeColumnToContents(column);

  emit SelectionChanged();
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

void QmitkCrashDumpListWidget::FitHeightToRows()
{
  int height = m_Tree->header()->sizeHint().height() + 2 * m_Tree->frameWidth();

  for (int row = 0; row < m_Tree->topLevelItemCount(); ++row)
    height += m_Tree->sizeHintForRow(row);

  // Room for a horizontal scroll bar, should the columns not fit.
  height += m_Tree->horizontalScrollBar()->sizeHint().height();

  m_Tree->setFixedHeight(height);
  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void QmitkCrashDumpListWidget::SetSelectionEnabled(bool enabled)
{
  m_Tree->setSelectionMode(enabled ? QAbstractItemView::ExtendedSelection : QAbstractItemView::NoSelection);
  m_Tree->setFocusPolicy(enabled ? Qt::StrongFocus : Qt::NoFocus);
}
