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

#include "QmitkUSControlsCustomDiPhASDeviceWidget.h"
#include "ui_QmitkUSControlsCustomDiPhASDeviceWidget.h"
#include <QMessageBox>

#include <mitkException.h>

QmitkUSControlsCustomDiPhASDeviceWidget::QmitkUSControlsCustomDiPhASDeviceWidget(QWidget *parent)
  : QmitkUSAbstractCustomWidget(parent), ui(new Ui::QmitkUSControlsCustomDiPhASDeviceWidget)
{
}

QmitkUSControlsCustomDiPhASDeviceWidget::~QmitkUSControlsCustomDiPhASDeviceWidget()
{
  delete ui;
}

std::string QmitkUSControlsCustomDiPhASDeviceWidget::GetDeviceClass() const
{
  return "org.mitk.modules.us.USDiPhASDevice";
}

QmitkUSAbstractCustomWidget* QmitkUSControlsCustomDiPhASDeviceWidget::Clone(QWidget* parent) const
{
  QmitkUSAbstractCustomWidget* clonedWidget = new QmitkUSControlsCustomDiPhASDeviceWidget(parent);
  clonedWidget->SetDevice(this->GetDevice());
  return clonedWidget;
}

void QmitkUSControlsCustomDiPhASDeviceWidget::OnDeviceSet()
{
  m_ControlInterface = dynamic_cast<mitk::USDiPhASDeviceCustomControls*>
    (this->GetDevice()->GetControlInterfaceCustom().GetPointer());

  if ( m_ControlInterface.IsNotNull() )
  {
    //ui->SpeedOfSound->setValue(cropping.left);
  }
  else
  {
    MITK_WARN("QmitkUSAbstractCustomWidget")("QmitkUSControlsCustomDiPhASDeviceWidget")
        << "Did not get a custom video device control interface.";
  }

  ui->SpeedOfSound->setEnabled(m_ControlInterface.IsNotNull());
}

void QmitkUSControlsCustomDiPhASDeviceWidget::Initialize()
{
  ui->setupUi(this);

  connect(ui->SpeedOfSound, SIGNAL(valueChanged(int)), this, SLOT(OnSpeedOfSoundChanged()));
}

void QmitkUSControlsCustomDiPhASDeviceWidget::OnSpeedOfSoundChanged()
{
  if (m_ControlInterface.IsNull()) { return; }
  m_ControlInterface->SetSoundOfSpeed(ui->SpeedOfSound->value());

  MITK_INFO << "wrks";
}
