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
#include <QMessageBox>

#include <mitkException.h>

QmitkUSControlsCustomVideoDeviceWidget::QmitkUSControlsCustomVideoDeviceWidget(QWidget *parent)
  : QmitkUSAbstractCustomWidget(parent), ui(new Ui::QmitkUSControlsCustomVideoDeviceWidget)
{
  m_Cropping.left = 0;
  m_Cropping.top = 0;
  m_Cropping.right = 0;
  m_Cropping.bottom = 0;
}

QmitkUSControlsCustomVideoDeviceWidget::~QmitkUSControlsCustomVideoDeviceWidget()
{
  delete ui;
}

std::string QmitkUSControlsCustomVideoDeviceWidget::GetDeviceClass() const
{
  return mitk::USVideoDevice::GetDeviceClassStatic();
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

  if (m_ControlInterface.IsNotNull())
  {
    mitk::USImageVideoSource::USImageCropping cropping = m_ControlInterface->GetCropArea();
    ui->crop_left->setValue(cropping.left);
    ui->crop_right->setValue(cropping.right);
    ui->crop_bot->setValue(cropping.bottom);
    ui->crop_top->setValue(cropping.top);

    //get all probes and put their names into a combobox
    std::vector<mitk::USProbe::Pointer> probes = m_ControlInterface->GetProbes();
    for (std::vector<mitk::USProbe::Pointer>::iterator it = probes.begin(); it != probes.end(); it++)
    {
      std::string probeName = (*it)->GetName();
      ui->m_ProbeIdentifier->addItem(QString::fromUtf8(probeName.data(), probeName.size()));
    }
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

void QmitkUSControlsCustomVideoDeviceWidget::Initialize()
{
  ui->setupUi(this);

  connect(ui->crop_left, SIGNAL(valueChanged(int)), this, SLOT(OnCropAreaChanged()));
  connect(ui->crop_right, SIGNAL(valueChanged(int)), this, SLOT(OnCropAreaChanged()));
  connect(ui->crop_top, SIGNAL(valueChanged(int)), this, SLOT(OnCropAreaChanged()));
  connect(ui->crop_bot, SIGNAL(valueChanged(int)), this, SLOT(OnCropAreaChanged()));
  connect(ui->m_UsDepth, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnDepthChanged()));
  connect(ui->m_ProbeIdentifier, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnProbeChanged()));
}

void QmitkUSControlsCustomVideoDeviceWidget::OnCropAreaChanged()
{
  if (m_ControlInterface.IsNull()) { return; }

  mitk::USImageVideoSource::USImageCropping cropping;
  cropping.left = ui->crop_left->value();
  cropping.top = ui->crop_top->value();
  cropping.right = ui->crop_right->value();
  cropping.bottom = ui->crop_bot->value();

  try
  {
    m_ControlInterface->SetCropArea(cropping);
    m_Cropping = cropping;
  }
  catch (mitk::Exception e)
  {
    m_ControlInterface->SetCropArea(m_Cropping); // reset to last valid crop

    //reset values
    BlockSignalAndSetValue(ui->crop_left, m_Cropping.left);
    BlockSignalAndSetValue(ui->crop_right, m_Cropping.right);
    BlockSignalAndSetValue(ui->crop_top, m_Cropping.top);
    BlockSignalAndSetValue(ui->crop_bot, m_Cropping.bottom);

    // inform user
    QMessageBox msgBox;
    msgBox.setInformativeText("The crop area you specified is invalid.\nPlease make sure that no more pixels are cropped than are available.");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
    MITK_WARN << "User tried to crop beyond limits of the image";
  }
}

void QmitkUSControlsCustomVideoDeviceWidget::OnDepthChanged()
{
  double depth = ui->m_UsDepth->currentText().toDouble();
  m_ControlInterface->SetNewDepth(depth);
}

void QmitkUSControlsCustomVideoDeviceWidget::OnProbeChanged()
{
  std::string probename = ui->m_ProbeIdentifier->currentText().toStdString();
  m_ControlInterface->SetNewProbeIdentifier(probename);
  SetDepthsForProbe(probename);
}

void QmitkUSControlsCustomVideoDeviceWidget::BlockSignalAndSetValue(QSpinBox* target, int value)
{
  bool oldState = target->blockSignals(true);
  target->setValue(value);
  target->blockSignals(oldState);
}

void QmitkUSControlsCustomVideoDeviceWidget::SetDepthsForProbe(std::string probename)
{
  ui->m_UsDepth->clear();
  std::vector<int> depths = m_ControlInterface->GetDepthsForProbe(probename);
  for (std::vector<int>::iterator it = depths.begin(); it != depths.end(); it++)
  {
    ui->m_UsDepth->addItem(QString::number(*it));
  }
}
