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

#include <QmitkC3xyData.h>

QmitkC3xyData::QmitkC3xyData()
{
}

QmitkC3xyData::QmitkC3xyData(QMap<QVariant, QVariant> data) {
  SetData(data);
}

QmitkC3xyData::QmitkC3xyData(QList<QVariant> data) {
  QMap<QVariant, QVariant> augmentedData;
  unsigned int count = 0;
  //just augment the 1D data
  for (const auto& ele : data) {
    augmentedData[count] = ele;
    count++;
  }
  SetData(augmentedData);
}


void QmitkC3xyData::SetData(QMap<QVariant, QVariant> data)
{
	for (const auto& entry : data.toStdMap())
	{
		m_XData.push_back(entry.first);
		m_YData.push_back(entry.second);
	}
}

void QmitkC3xyData::ClearData()
{
  this->m_YData.clear();
  this->m_XData.clear();
}