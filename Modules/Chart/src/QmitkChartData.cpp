/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkChartData.h>

QmitkChartData::QmitkChartData()
  : m_ShowSubchart(true)
  , m_YAxisScale("")
  , m_StackedData(false)
{
}

void QmitkChartData::SetAppearance(bool showSubChart, bool usePercentageInPieChart)
{
  m_ShowSubchart = showSubChart;
  m_UsePercentageInPieChart = usePercentageInPieChart;
}
