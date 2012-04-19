/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-20 13:35:09 +0200 (Mi, 20 Mai 2009) $
Version:   $Revision: 17332 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

//#define _USE_MATH_DEFINES
#include <QmitkToFPMDParameterWidget.h>

//QT headers
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qcombobox.h>

//itk headers
#include <itksys/SystemTools.hxx>

const std::string QmitkToFPMDParameterWidget::VIEW_ID = "org.mitk.views.qmitktofpmdparameterwidget";

QmitkToFPMDParameterWidget::QmitkToFPMDParameterWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  this->m_IntegrationTime = 0;
  this->m_ModulationFrequency = 0;
  this->m_ToFImageGrabber = NULL;

  m_Controls = NULL;
  CreateQtPartControl(this);
}

QmitkToFPMDParameterWidget::~QmitkToFPMDParameterWidget()
{
}

void QmitkToFPMDParameterWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkToFPMDParameterWidgetControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();
  }
}

void QmitkToFPMDParameterWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->m_IntegrationTimeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnChangeIntegrationTimeSpinBox(int)) );
    connect( m_Controls->m_ModulationFrequencySpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnChangeModulationFrequencySpinBox(int)) );
  }
}

mitk::ToFImageGrabber* QmitkToFPMDParameterWidget::GetToFImageGrabber()
{
  return this->m_ToFImageGrabber;
}

void QmitkToFPMDParameterWidget::SetToFImageGrabber(mitk::ToFImageGrabber* aToFImageGrabber)
{
  this->m_ToFImageGrabber = aToFImageGrabber;
}

void QmitkToFPMDParameterWidget::ActivateAllParameters()
{
    this->m_IntegrationTime = m_Controls->m_IntegrationTimeSpinBox->value();
    this->m_IntegrationTime = this->m_ToFImageGrabber->SetIntegrationTime(this->m_IntegrationTime);

    this->m_ModulationFrequency = m_Controls->m_ModulationFrequencySpinBox->value();
    this->m_ModulationFrequency = this->m_ToFImageGrabber->SetModulationFrequency(this->m_ModulationFrequency);

    //set the PMD calibration according to the check boxes
    bool boolValue = false;
    boolValue = m_Controls->m_FPNCalibCB->isChecked();
    this->m_ToFImageGrabber->SetBoolProperty("SetFPNCalibration", boolValue);
    boolValue = m_Controls->m_FPPNCalibCB->isChecked();
    this->m_ToFImageGrabber->SetBoolProperty("SetFPPNCalibration", boolValue);
    boolValue = m_Controls->m_LinearityCalibCB->isChecked();
    this->m_ToFImageGrabber->SetBoolProperty("SetLinearityCalibration", boolValue);
    boolValue = m_Controls->m_LensCorrection->isChecked();
    this->m_ToFImageGrabber->SetBoolProperty("SetLensCalibration", boolValue);
    boolValue = m_Controls->m_ExposureModeCB->isChecked();
    this->m_ToFImageGrabber->SetBoolProperty("SetExposureMode", boolValue);

    //reset the GUI elements
    m_Controls->m_IntegrationTimeSpinBox->setValue(this->m_IntegrationTime);
    m_Controls->m_ModulationFrequencySpinBox->setValue(this->m_ModulationFrequency);
}

void QmitkToFPMDParameterWidget::OnChangeIntegrationTimeSpinBox(int value)
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

void QmitkToFPMDParameterWidget::OnChangeModulationFrequencySpinBox(int value)
{
  if (this->m_ToFImageGrabber != NULL)
  {
    // stop camera if active
    bool active = m_ToFImageGrabber->IsCameraActive();
    if (active)
    {
      m_ToFImageGrabber->StopCamera();
    }
    this->m_ModulationFrequency = m_Controls->m_ModulationFrequencySpinBox->value();
    this->m_ModulationFrequency = this->m_ToFImageGrabber->SetModulationFrequency(this->m_ModulationFrequency);
    if (active)
    {
      m_ToFImageGrabber->StartCamera();
    }
  }
}

