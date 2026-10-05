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
  m_Tree->setTextElideMode(Qt::ElideMiddle);
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
    item->setText(SizeColumn, QLocale().formattedDataSize(static_cast<qint64>(dump.SizeInBytes)));
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

void QmitkCrashDumpListWidget::SetColumnHidden(Column column, bool hidden)
{
  m_Tree->setColumnHidden(column, hidden);
}

void QmitkCrashDumpListWidget::FitToContents()
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

  int height = header->sizeHint().height() + frame;
  for (int row = 0; row < m_Tree->topLevelItemCount(); ++row)
    height += m_Tree->sizeHintForRow(row);

  m_Tree->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_Tree->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_Tree->setMinimumWidth(header->length() + frame);
  m_Tree->setFixedHeight(height);
  header->setSectionResizeMode(VersionColumn, QHeaderView::Stretch);
  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void QmitkCrashDumpListWidget::SetSelectionEnabled(bool enabled)
{
  m_Tree->setSelectionMode(enabled ? QAbstractItemView::ExtendedSelection : QAbstractItemView::NoSelection);
  m_Tree->setFocusPolicy(enabled ? Qt::StrongFocus : Qt::NoFocus);
}
