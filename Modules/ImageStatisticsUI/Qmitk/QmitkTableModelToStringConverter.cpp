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

#include "QmitkTableModelToStringConverter.h"

#include <mitkCommon.h>

QmitkTableModelToStringConverter::QmitkTableModelToStringConverter() 
{
}

void QmitkTableModelToStringConverter::SetTableModel(QAbstractItemModel* model)
{
  m_tableModel = model;
}

QString QmitkTableModelToStringConverter::GetString() const
{
  if (m_tableModel == nullptr) {
    mitkThrow() << "Cannot convert TableModel to String: TableModel is nullptr";
  }

  if (m_transposeOutput) {
    return GetStringTransposed();
  }
  else {
    return GetStringNonTransposed();
  }
}

void QmitkTableModelToStringConverter::SetTransposeOutput(bool transposeOutput)
{
  m_transposeOutput = transposeOutput;
}

void QmitkTableModelToStringConverter::SetLineDelimiter(QChar lineDelimiter)
{
  m_lineDelimiter = lineDelimiter;
}

void QmitkTableModelToStringConverter::SetColumnDelimiter(QChar columnDelimiter)
{
  m_columnDelimiterWithSpace = columnDelimiter + QString(" ");
}

void QmitkTableModelToStringConverter::SetIncludeHeaderData(bool includeHeaderData)
{
  m_includeHeaderData = includeHeaderData;
}

QString QmitkTableModelToStringConverter::GetStringTransposed() const
{
  QString textData;
  int rows = m_tableModel->rowCount();
  int columns = m_tableModel->columnCount();

  if (m_includeHeaderData) {
    textData += "  ";
    for (int i = 0; i < rows; i++) {
      textData += m_columnDelimiterWithSpace;
      textData += m_tableModel->headerData(i, Qt::Vertical).toString();
    }
    textData += m_lineDelimiter;
  }

  for (int i = 0; i < columns; i++) {
    if (i > 0) {
      textData += m_lineDelimiter;
    }
    if (m_includeHeaderData) {
      textData += m_tableModel->headerData(i, Qt::Horizontal).toString() + m_columnDelimiterWithSpace;
    }
    for (int j = 0; j < rows; j++) {
      if (j > 0) {
        textData += m_columnDelimiterWithSpace;
      }
      textData += m_tableModel->data(m_tableModel->index(j, i)).toString();
      MITK_WARN << i << " " << j;
    }
  }
  return textData;
}

QString QmitkTableModelToStringConverter::GetStringNonTransposed() const
{
  QString textData;
  int rows = m_tableModel->rowCount();
  int columns = m_tableModel->columnCount();

  if (m_includeHeaderData) {
    textData += "  ";
    for (int i = 0; i < columns; i++) {
      textData += m_columnDelimiterWithSpace;
      textData += m_tableModel->headerData(i, Qt::Horizontal).toString();
    }
    textData += m_lineDelimiter;
  }

  for (int i = 0; i < rows; i++) {
    if (i > 0) {
      textData += m_lineDelimiter;
    }
    if (m_includeHeaderData) {
      textData += m_tableModel->headerData(i, Qt::Vertical).toString() + m_columnDelimiterWithSpace;
    }
    for (int j = 0; j < columns; j++) {
      if (j > 0) {
        textData += m_columnDelimiterWithSpace;
      }
      textData += m_tableModel->data(m_tableModel->index(i, j)).toString();
    }
  }
  return textData;
}
