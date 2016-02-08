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

#include "QmitkVirtualTrackerWidget.h"

#include "mitkVirtualTrackingDevice.h"

const std::string QmitkVirtualTrackerWidget::VIEW_ID = "org.mitk.views.VirtualTrackerWidget";

QmitkVirtualTrackerWidget::QmitkVirtualTrackerWidget(QWidget* parent, Qt::WindowFlags f)
  : QmitkAbstractTrackingDeviceWidget(parent, f)
  , m_Controls(nullptr)
{
}

void QmitkVirtualTrackerWidget::Initialize()
{
  InitializeSuperclassWidget();
  CreateQtPartControl(this);
  CreateConnections();
}

QmitkVirtualTrackerWidget::~QmitkVirtualTrackerWidget()
{
  delete m_Controls;
}

void QmitkVirtualTrackerWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkVirtualTrackerWidget;
    m_Controls->setupUi(parent);
  }
}

void QmitkVirtualTrackerWidget::CreateConnections()
{
  if (m_Controls)
  {
    connect((QObject*)(m_Controls->m_EnableGaussianNoise), SIGNAL(clicked()), this, SLOT(EnableGaussianNoise()));
  }
}

mitk::TrackingDevice::Pointer QmitkVirtualTrackerWidget::ConstructTrackingDevice()
{
  // Create the Virtual Tracking Device
  mitk::VirtualTrackingDevice::Pointer returnValue = mitk::VirtualTrackingDevice::New();
  if (m_Controls->m_EnableGaussianNoise->isChecked())
  {
    returnValue->EnableGaussianNoise();
    returnValue->SetParamsForGaussianNoise(m_Controls->m_MeanDistributionParam->value(), m_Controls->m_DeviationDistributionParam->value());
  }
  return static_cast<mitk::TrackingDevice::Pointer>(returnValue); //static_cast necessary for compiling with Linux
}

void QmitkVirtualTrackerWidget::EnableGaussianNoise()
{
  if (m_Controls->m_EnableGaussianNoise->isChecked())
  {
    m_Controls->m_MeanDistributionParam->setEnabled(true);
    m_Controls->m_DeviationDistributionParam->setEnabled(true);
  }
  else
  {
    m_Controls->m_MeanDistributionParam->setEnabled(false);
    m_Controls->m_DeviationDistributionParam->setEnabled(false);
  }
}

QmitkVirtualTrackerWidget* QmitkVirtualTrackerWidget::Clone(QWidget* parent) const
{
  QmitkVirtualTrackerWidget* clonedWidget = new QmitkVirtualTrackerWidget(parent);
  clonedWidget->Initialize();

  clonedWidget->m_Controls->m_EnableGaussianNoise->setEnabled(m_Controls->m_EnableGaussianNoise->isEnabled());
  clonedWidget->m_Controls->m_MeanDistributionParam->setValue(m_Controls->m_MeanDistributionParam->value());
  clonedWidget->m_Controls->m_DeviationDistributionParam->setValue(m_Controls->m_DeviationDistributionParam->value());
  return clonedWidget;
}
