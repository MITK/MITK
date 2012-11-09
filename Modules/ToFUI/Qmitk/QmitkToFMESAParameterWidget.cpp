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

//#define _USE_MATH_DEFINES
#include <QmitkToFMESAParameterWidget.h>

//QT headers
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qcombobox.h>

//itk headers
#include <itksys/SystemTools.hxx>

const std::string QmitkToFMESAParameterWidget::VIEW_ID = "org.mitk.views.qmitktofpmdparameterwidget";

QmitkToFMESAParameterWidget::QmitkToFMESAParameterWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  this->m_IntegrationTime = 0;
  this->m_ModulationFrequency = 0;
  this->m_ToFImageGrabber = NULL;

  m_Controls = NULL;
  CreateQtPartControl(this);
}

QmitkToFMESAParameterWidget::~QmitkToFMESAParameterWidget()
{
}

void QmitkToFMESAParameterWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkToFMESAParameterWidgetControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();
  }
}

void QmitkToFMESAParameterWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->m_IntegrationTimeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnChangeIntegrationTimeSpinBox(int)) );
    connect( m_Controls->m_ModulationFrequencyComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnChangeModulationFrequencyComboBox(int)) );
    connect( m_Controls->m_FPNCB, SIGNAL(toggled(bool)), this, SLOT(OnChangeFPNCheckBox(bool)) );
    connect( m_Controls->m_ConvGrayCB, SIGNAL(toggled(bool)), this, SLOT(OnChangeConvGrayCheckBox(bool)) );
    connect( m_Controls->m_MedianCB, SIGNAL(toggled(bool)), this, SLOT(OnChangeMedianCheckBox(bool)) );
    connect( m_Controls->m_ANFCB, SIGNAL(toggled(bool)), this, SLOT(OnChangeANFCheckBox(bool)) );
  }
}

mitk::ToFImageGrabber* QmitkToFMESAParameterWidget::GetToFImageGrabber()
{
  return this->m_ToFImageGrabber;
}

void QmitkToFMESAParameterWidget::SetToFImageGrabber(mitk::ToFImageGrabber* aToFImageGrabber)
{
  this->m_ToFImageGrabber = aToFImageGrabber;
}

void QmitkToFMESAParameterWidget::ActivateAllParameters()
{
    this->m_IntegrationTime = m_Controls->m_IntegrationTimeSpinBox->value();
    this->m_IntegrationTime = this->m_ToFImageGrabber->SetIntegrationTime(this->m_IntegrationTime);

    switch(m_Controls->m_ModulationFrequencyComboBox->currentIndex())
    {
    case 0: this->m_ModulationFrequency = 29; break;
    case 1: this->m_ModulationFrequency = 30; break;
    case 2: this->m_ModulationFrequency = 31; break;
    default: this->m_ModulationFrequency = 30; break;
    }
    this->m_ModulationFrequency = this->m_ToFImageGrabber->SetModulationFrequency(this->m_ModulationFrequency);

    //set the MESA acquire mode according to the check boxes
    bool boolValue = false;
    boolValue = m_Controls->m_FPNCB->isChecked();
    this->m_ToFImageGrabber->SetBoolProperty("SetFPN", boolValue);
    boolValue = m_Controls->m_ConvGrayCB->isChecked();
    this->m_ToFImageGrabber->SetBoolProperty("SetConvGray", boolValue);
    boolValue = m_Controls->m_MedianCB->isChecked();
    this->m_ToFImageGrabber->SetBoolProperty("SetMedian", boolValue);
    boolValue = m_Controls->m_ANFCB->isChecked();
    this->m_ToFImageGrabber->SetBoolProperty("SetANF", boolValue);

    //reset the GUI elements
    m_Controls->m_IntegrationTimeSpinBox->setValue(this->m_IntegrationTime);
    //m_Controls->m_ModulationFrequencyComboBox->setValue(this->m_ModulationFrequency);
}

void QmitkToFMESAParameterWidget::OnChangeFPNCheckBox(bool checked)
{
  this->m_ToFImageGrabber->SetBoolProperty("SetFPN", checked);
}

void QmitkToFMESAParameterWidget::OnChangeConvGrayCheckBox(bool checked)
{
  this->m_ToFImageGrabber->SetBoolProperty("SetConvGray", checked);
}

void QmitkToFMESAParameterWidget::OnChangeMedianCheckBox(bool checked)
{
  this->m_ToFImageGrabber->SetBoolProperty("SetMedian", checked);
}

void QmitkToFMESAParameterWidget::OnChangeANFCheckBox(bool checked)
{
  this->m_ToFImageGrabber->SetBoolProperty("SetANF", checked);
}

void QmitkToFMESAParameterWidget::OnChangeIntegrationTimeSpinBox(int value)
{
  if (this->m_ToFImageGrabber != NULL)
  {
    // stop camera if active
    bool active = m_ToFImageGrabber->IsCameraActive();
    if (active)
    {
      m_ToFImageGrabber->StopCamera();
    }
    this->m_IntegrationTime = m_Controls->m_IntegrationTimeSpinBox->value();
    this->m_IntegrationTime = this->m_ToFImageGrabber->SetIntegrationTime(this->m_IntegrationTime);
    if (active)
    {
      m_ToFImageGrabber->StartCamera();
    }
  }
}

void QmitkToFMESAParameterWidget::OnChangeModulationFrequencyComboBox(int index)
{
  if (this->m_ToFImageGrabber != NULL)
  {
    // stop camera if active
    bool active = m_ToFImageGrabber->IsCameraActive();
    if (active)
    {
      m_ToFImageGrabber->StopCamera();
    }
    switch(index)
    {
    case 0: this->m_ModulationFrequency = 29; break;
    case 1: this->m_ModulationFrequency = 30; break;
    case 2: this->m_ModulationFrequency = 31; break;
    default: this->m_ModulationFrequency = 30; break;
    }
    this->m_ModulationFrequency = this->m_ToFImageGrabber->SetModulationFrequency(this->m_ModulationFrequency);
    if (active)
    {
      m_ToFImageGrabber->StartCamera();
    }
  }
}

