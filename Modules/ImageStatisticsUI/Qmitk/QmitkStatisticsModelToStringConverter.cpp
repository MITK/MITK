/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkStatisticsModelToStringConverter.h"
#include "mitkExceptionMacro.h"

QmitkStatisticsModelToStringConverter::QmitkStatisticsModelToStringConverter() {}

void QmitkStatisticsModelToStringConverter::SetTableModel(QAbstractItemModel *model)
{
  m_statisticsModel = model;
}

void QmitkStatisticsModelToStringConverter::SetRootIndex(QModelIndex rootIndex)
{
  m_rootIndex = rootIndex;
}

QString QmitkStatisticsModelToStringConverter::GetString() const
{
  if (m_statisticsModel == nullptr)
  {
    mitkThrow() << "Cannot convert TableModel to String: TableModel is nullptr";
  }

  QString textData;
  int columns = m_statisticsModel->columnCount();

  if (m_includeHeaderData)
  {
    for (int i = 0; i < columns; i++)
    {
      if (i > 0)
      {
        textData += m_columnDelimiter;
      }
      textData += m_statisticsModel->headerData(i, Qt::Horizontal).toString();
    }
    textData += m_rowDelimiter;
  }
  textData += Iterate(m_rootIndex, m_statisticsModel);

  return textData;
}

void QmitkStatisticsModelToStringConverter::SetRowDelimiter(QChar rowDelimiter)
{
  m_rowDelimiter = rowDelimiter;
}

void QmitkStatisticsModelToStringConverter::SetColumnDelimiter(QChar columnDelimiter)
{
  m_columnDelimiter = columnDelimiter;
}

void QmitkStatisticsModelToStringConverter::SetIncludeHeaderData(bool includeHeaderData)
{
  m_includeHeaderData = includeHeaderData;
}

QString QmitkStatisticsModelToStringConverter::Iterate(const QModelIndex &index,
                                                       const QAbstractItemModel *model,
                                                       int depth) const
{
  QString content;
  if (index.isValid())
  {
    content = index.data().toString();
  }
  if (!model->hasChildren(index) || (index.flags() & Qt::ItemNeverHasChildren))
  {
    return content;
  }
  else
  {
    content += m_rowDelimiter;
  }

  auto rows = model->rowCount(index);
  auto cols = model->columnCount(index);
  for (int i = 0; i < rows; ++i)
  {
    if (i > 0)
    {
      content += m_rowDelimiter;
    }
    for (int j = 0; j < cols; ++j)
    {
      if (j > 0)
      {
        content += m_columnDelimiter;
      }
      content += Iterate(model->index(i, j, index), model, depth + 1);
    }
  }
  return content;
}
