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

#include <QmitkC3Data.h>

QmitkC3Data::QmitkC3Data()
{
}

QmitkC3Data::QmitkC3Data(HistogramType::ConstPointer histogram)
  : m_Histogram(histogram),
  m_UseLineChart(false),
  m_ShowSubchart(true)
{
}

void QmitkC3Data::SetAppearance(bool UseLineChart = false, bool ShowSubChart = true)
{
  m_UseLineChart = UseLineChart;
  m_ShowSubchart = ShowSubChart;
}

void QmitkC3Data::ClearData()
{
  this->m_YData.clear();
  this->m_XData.clear();
}