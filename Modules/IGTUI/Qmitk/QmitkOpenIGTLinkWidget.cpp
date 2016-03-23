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

#include "QmitkOpenIGTLinkWidget.h"

#include "mitkOpenIGTLinkTrackingDevice.h"

const std::string QmitkOpenIGTLinkWidget::VIEW_ID = "org.mitk.views.OpenIGTLinkWidget";

QmitkOpenIGTLinkWidget::QmitkOpenIGTLinkWidget(QWidget* parent, Qt::WindowFlags f)
  : QmitkAbstractTrackingDeviceWidget(parent, f)
  , m_Controls(nullptr)
{
}

void QmitkOpenIGTLinkWidget::Initialize()
{
  InitializeSuperclassWidget();
  CreateQtPartControl(this);
}

QmitkOpenIGTLinkWidget::~QmitkOpenIGTLinkWidget()
{
  delete m_Controls;
}

void QmitkOpenIGTLinkWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkOpenIGTLinkWidget;
    m_Controls->setupUi(parent);
  }
}

mitk::TrackingDevice::Pointer QmitkOpenIGTLinkWidget::ConstructTrackingDevice()
{
  // Create the Virtual Tracking Device
  mitk::OpenIGTLinkTrackingDevice::Pointer OIGTLDevice = mitk::OpenIGTLinkTrackingDevice::New();
  OIGTLDevice->SetPortNumber(m_Controls->m_OpenIGTLinkPort->text().toInt());
  OIGTLDevice->SetHostname(m_Controls->m_OpenIGTLinkHostname->text().toStdString());
  OIGTLDevice->SetUpdateRate(m_Controls->m_UpdateRate->value());
  return static_cast<mitk::TrackingDevice::Pointer>(OIGTLDevice);
}

QmitkOpenIGTLinkWidget* QmitkOpenIGTLinkWidget::Clone(QWidget* parent) const
{
  QmitkOpenIGTLinkWidget* clonedWidget = new QmitkOpenIGTLinkWidget(parent);
  clonedWidget->Initialize();

  clonedWidget->m_Controls->m_OpenIGTLinkPort->setText(m_Controls->m_OpenIGTLinkPort->text());
  clonedWidget->m_Controls->m_OpenIGTLinkHostname->setText(m_Controls->m_OpenIGTLinkHostname->text());

  return clonedWidget;
}
