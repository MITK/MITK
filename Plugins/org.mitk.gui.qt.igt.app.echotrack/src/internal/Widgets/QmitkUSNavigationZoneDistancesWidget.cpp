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

#include "QmitkUSNavigationZoneDistancesWidget.h"
#include "QmitkZoneProgressBar.h"

#include "mitkDataNode.h"
#include "mitkNavigationData.h"

#include <QVBoxLayout>

QmitkUSNavigationZoneDistancesWidget::QmitkUSNavigationZoneDistancesWidget(QWidget *parent) :
  QWidget(parent),
  m_SizePropertyKey("zone.size")
{
  this->setLayout(new QVBoxLayout(this));
}

QmitkUSNavigationZoneDistancesWidget::~QmitkUSNavigationZoneDistancesWidget()
{
}

void QmitkUSNavigationZoneDistancesWidget::SetSizePropertyKey(const std::string& sizePropertyKey)
{
  m_SizePropertyKey = sizePropertyKey;
}

const std::string& QmitkUSNavigationZoneDistancesWidget::GetSizePropertyKey() const
{
  return m_SizePropertyKey;
}

void QmitkUSNavigationZoneDistancesWidget::AddZone(itk::SmartPointer<mitk::DataNode> zoneNode)
{
  m_ZoneNodes.append(zoneNode);
  QmitkZoneProgressBar* progressBar =
    new QmitkZoneProgressBar(QString::fromStdString(zoneNode->GetName())+QString(": %1 mm"), 60, 25, this);

  float color[3] = {1, 0, 0};
  progressBar->SetWarnColor(color);

  zoneNode->GetColor(color);
  progressBar->SetColor(color);

  m_ZoneProgressBars.append(progressBar);
  this->layout()->addWidget(progressBar);
}

void QmitkUSNavigationZoneDistancesWidget::ClearZones()
{
  // clear risk zones
  for (QVector<QmitkZoneProgressBar*>::Iterator it = m_ZoneProgressBars.begin();
       it != m_ZoneProgressBars.end(); ++it)
  {
    this->layout()->removeWidget(*it);
    delete *it;
  }

  m_ZoneProgressBars.clear();
  m_ZoneNodes.clear();
}

void QmitkUSNavigationZoneDistancesWidget::UpdateDistancesToNeedlePosition(mitk::NavigationData::Pointer needle)
{
  if ( needle.IsNull() )
  {
    MITK_ERROR("QmitkUSAbstractNavigationStep")("QmitkUSNavigationStepMarkerIntervention")
      << "Current Navigation Data for needle must not be null.";
    mitkThrow() << "Current Navigation Data for needle must not be null.";
  }

  // get needle position
  if (needle->IsDataValid())
  {
    mitk::Point3D needlePosition = needle->GetPosition();

    for (int n = 0; n < m_ZoneNodes.size(); ++n)
    {
      mitk::Point3D zoneOrigin = m_ZoneNodes.at(n)->GetData()->GetGeometry()->GetOrigin();

      // calculate absolute distance
      mitk::ScalarType distance = sqrt( pow(zoneOrigin[0] - needlePosition[0], 2) + pow(zoneOrigin[1] - needlePosition[1], 2) + pow(zoneOrigin[2] - needlePosition[2], 2) );

      // subtract zone size
      float zoneSize;
      m_ZoneNodes.at(n)->GetFloatProperty(m_SizePropertyKey.c_str(), zoneSize);
      distance = distance - zoneSize;

      m_ZoneProgressBars.at(n)->setValue(distance);

      if ( distance < 0 ) { SignalZoneViolated(m_ZoneNodes.at(n), needlePosition); }
    }
  }
}
