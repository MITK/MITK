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

void QmitkStatisticsModelToStringConverter::SetModel(QmitkImageStatisticsTreeModel *model)
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
      textData += m_statisticsModel->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
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
                                                       const QmitkImageStatisticsTreeModel *model,
                                                       QString label) const
{
  QString content;

  if (model->hasChildren(index))
  {
    if (index.isValid())
    {
      label += index.data().toString() + QString(" >> ");
    }

    auto rows = model->rowCount(index);
    for (int r = 0; r < rows; ++r)
    {
      auto childIndex = model->index(r, 0, index);

      if (model->hasChildren(childIndex))
      {
        content += Iterate(childIndex, model, label);
      }
      else
      {
        content += label;

        auto cols = model->columnCount(index);
        for (int c = 0; c < cols; ++c)
        {
          if (c > 0)
          {
            content += m_columnDelimiter;
          }

          auto columnChildIndex = model->index(r, c, index);
          content += Iterate(columnChildIndex, model, label);
        }
        content += m_rowDelimiter;
      }
    }
  }
  else
  {
    if (index.isValid())
    {
      auto data = index.data();
      if (static_cast<QMetaType::Type>(data.type()) == QMetaType::Double)
      {
        content = QString("%L1").arg(data.toDouble(), 0, 'f');
      }
      else
      {
        content = data.toString();
      }
    }
  }

  return content;
}
