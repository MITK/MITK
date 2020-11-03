/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkChartxyData.h>

QmitkChartxyData::QmitkChartxyData(const std::vector< std::pair<double, double> > &data,
                                   const QVariant &label,
                                   const QVariant &chartType,
                                   const QVariant &position)
  : m_LabelCount(position), m_Label(label), m_ChartType(chartType)
{
  SetData(data);
}

void QmitkChartxyData::SetData(const std::vector< std::pair<double, double> > &data)
{
  ClearData();
  for (const auto &entry : data)
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
