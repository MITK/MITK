/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
        textData += m_columnDelimiterWithSpace;
      }
      textData += m_statisticsModel->headerData(i, Qt::Horizontal).toString();
    }
    textData += m_lineDelimiter;
  }
  textData += Iterate(m_rootIndex, m_statisticsModel);

  return textData;
}

void QmitkStatisticsModelToStringConverter::SetRowDelimiter(QChar lineDelimiter)
{
  m_lineDelimiter = lineDelimiter;
}

void QmitkStatisticsModelToStringConverter::SetColumnDelimiter(QChar columnDelimiter)
{
  m_columnDelimiterWithSpace = columnDelimiter + QString(" ");
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
    content += m_lineDelimiter;
  }

  auto rows = model->rowCount(index);
  auto cols = model->columnCount(index);
  for (int i = 0; i < rows; ++i)
  {
    if (i > 0)
    {
      content += m_lineDelimiter;
    }
    for (int j = 0; j < cols; ++j)
    {
      if (j > 0)
      {
        content += m_columnDelimiterWithSpace;
      }
      content += Iterate(model->index(i, j, index), model, depth + 1);
    }
  }
  return content;
}
