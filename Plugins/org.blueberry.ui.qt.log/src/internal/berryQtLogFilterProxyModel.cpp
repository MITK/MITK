/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQtLogFilterProxyModel.h"
#include "berryQtPlatformLogModel.h"

namespace berry {

QtLogFilterProxyModel::QtLogFilterProxyModel(QObject* parent)
  : QSortFilterProxyModel(parent),
    m_MinimumLevel(mitk::LogLevel::Debug),
    m_ShowDetails(false)
{
}

QtLogFilterProxyModel::~QtLogFilterProxyModel()
{
}

void QtLogFilterProxyModel::SetMinimumLevel(mitk::LogLevel level)
{
  if (m_MinimumLevel == level)
    return;

  this->beginFilterChange();
  m_MinimumLevel = level;
  this->endFilterChange(QSortFilterProxyModel::Direction::Rows);
}

void QtLogFilterProxyModel::SetShowDetails(bool showDetails)
{
  if (m_ShowDetails == showDetails)
    return;

  this->beginFilterChange();
  m_ShowDetails = showDetails;
  this->endFilterChange(QSortFilterProxyModel::Direction::Rows);
}

bool QtLogFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
  auto* source = this->sourceModel();

  if (nullptr == source)
    return false;

  // The level check comes first: it is a single integer comparison, while the
  // text search builds a string per column.
  const auto level = source->index(sourceRow, 0, sourceParent).data(QtPlatformLogModel::LogLevelRole);

  if (level.isValid() &&
      QtPlatformLogModel::GetSeverityRank(static_cast<mitk::LogLevel>(level.toInt())) <
      QtPlatformLogModel::GetSeverityRank(m_MinimumLevel))
  {
    return false;
  }

  const auto pattern = this->filterRegularExpression();

  if (pattern.pattern().isEmpty())
    return true;

  for (int column = 0; column < QtPlatformLogModel::ColumnCount; ++column)
  {
    if (!m_ShowDetails &&
        QtPlatformLogModel::IsDetailColumn(static_cast<QtPlatformLogModel::Column>(column)))
    {
      continue;
    }

    const auto text = source->index(sourceRow, column, sourceParent).data(this->filterRole()).toString();

    if (text.contains(pattern))
      return true;
  }

  return false;
}

}
