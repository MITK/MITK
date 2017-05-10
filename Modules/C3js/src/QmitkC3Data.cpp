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

QmitkC3Data::QmitkC3Data() : m_DiagramTypeName(QVariant("line")),
m_ShowSubchart(true)
{
}

void QmitkC3Data::SetAppearance(QVariant diagramTypeName, bool showSubChart, bool usePercentageInPieChart)
{
  m_DiagramTypeName = diagramTypeName;
  m_ShowSubchart = showSubChart;
  m_UsePercentageInPieChart = usePercentageInPieChart;
}