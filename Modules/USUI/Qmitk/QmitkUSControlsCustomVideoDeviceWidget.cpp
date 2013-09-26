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

#include "QmitkUSControlsCustomVideoDeviceWidget.h"
#include "ui_QmitkUSControlsCustomVideoDeviceWidget.h"

#include "mitkUSVideoDevice.h"

QmitkUSControlsCustomVideoDeviceWidget::QmitkUSControlsCustomVideoDeviceWidget(QWidget *parent)
  : QmitkUSAbstractCustomWidget(parent), ui(new Ui::QmitkUSControlsCustomVideoDeviceWidget)
{
  ui->setupUi(this);

  connect( ui->crop_left, SIGNAL(valueChanged(int)), this, SLOT(OnCropAreaChanged()) );
  connect( ui->crop_right, SIGNAL(valueChanged(int)), this, SLOT(OnCropAreaChanged()) );
  connect( ui->crop_top, SIGNAL(valueChanged(int)), this, SLOT(OnCropAreaChanged()) );
  connect( ui->crop_bot, SIGNAL(valueChanged(int)), this, SLOT(OnCropAreaChanged()) );
}

QmitkUSControlsCustomVideoDeviceWidget::~QmitkUSControlsCustomVideoDeviceWidget()
{
  delete ui;
}

std::string QmitkUSControlsCustomVideoDeviceWidget::GetDeviceClass() const
{
  return mitk::USVideoDevice::DeviceClassIdentifier;
}

QmitkUSAbstractCustomWidget* QmitkUSControlsCustomVideoDeviceWidget::Clone(QWidget* parent) const
{
  QmitkUSAbstractCustomWidget* clonedWidget = new QmitkUSControlsCustomVideoDeviceWidget(parent);
  clonedWidget->SetDevice(this->GetDevice());
  return clonedWidget;
}

void QmitkUSControlsCustomVideoDeviceWidget::OnDeviceSet()
{
  m_ControlInterface = dynamic_cast<mitk::USVideoDeviceCustomControls*>
    (this->GetDevice()->GetControlInterfaceCustom().GetPointer());

  if ( m_ControlInterface.IsNotNull() )
  {
    mitk::USImageVideoSource::USImageCropping cropping = m_ControlInterface->GetCropArea();
    ui->crop_left->setValue(cropping.left);
    ui->crop_right->setValue(cropping.right);
    ui->crop_bot->setValue(cropping.bottom);
    ui->crop_top->setValue(cropping.top);
  }
  else
  {
    MITK_WARN("QmitkUSAbstractCustomWidget")("QmitkUSControlsCustomVideoDeviceWidget")
        << "Did not get a custom video device control interface.";
  }

  ui->crop_left->setEnabled(m_ControlInterface.IsNotNull());
  ui->crop_right->setEnabled(m_ControlInterface.IsNotNull());
  ui->crop_bot->setEnabled(m_ControlInterface.IsNotNull());
  ui->crop_top->setEnabled(m_ControlInterface.IsNotNull());
}

void QmitkUSControlsCustomVideoDeviceWidget::OnCropAreaChanged()
{
  if ( m_ControlInterface.IsNull() ) { return; }

  mitk::USImageVideoSource::USImageCropping cropping;
  cropping.left = ui->crop_left->value();
  cropping.top = ui->crop_top->value();
  cropping.right = ui->crop_right->value();
  cropping.bottom = ui->crop_bot->value();

  m_ControlInterface->SetCropArea(cropping);
}