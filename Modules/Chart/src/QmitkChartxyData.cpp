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

#include <QmitkChartxyData.h>

QmitkChartxyData::QmitkChartxyData(const QMap<QVariant, QVariant> &data,
                                   const QVariant &label,
                                   const QVariant &chartType,
                                   const QVariant &position)
  : m_LabelCount(position), m_Label(label), m_ChartType(chartType)
{
  SetData(data);
}

void QmitkChartxyData::SetData(const QMap<QVariant, QVariant> &data)
{
  ClearData();
  for (const auto &entry : data.toStdMap())
  {
    m_XData.push_back(entry.first);
    m_YData.push_back(entry.second);
  }
  emit SignalDataChanged(m_YData);
}

void QmitkChartxyData::ClearData()
{
  this->m_YData.clear();
  this->m_XData.clear();
  this->m_XErrorDataPlus.clear();
  this->m_XErrorDataMinus.clear();
  this->m_YErrorDataPlus.clear();
  this->m_YErrorDataMinus.clear();
}
