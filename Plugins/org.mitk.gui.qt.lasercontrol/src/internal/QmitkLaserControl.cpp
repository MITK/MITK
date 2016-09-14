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

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkLaserControl.h"

// Qt
#include <QMessageBox>

//mitk
#include <mitkOpotekPumpLaserController.h>

const std::string OPOLaserControl::VIEW_ID = "org.mitk.views.lasercontrol";

void OPOLaserControl::SetFocus()
{
  m_Controls.buttonConnect->setFocus();
}

void OPOLaserControl::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  connect(m_Controls.buttonConnect, SIGNAL(clicked()), this, SLOT(ConnectToLaser()));
  connect(m_Controls.buttonStatus, SIGNAL(clicked()), this, SLOT(GetStatus()));
  connect(m_Controls.buttonSendCustomMessage, SIGNAL(clicked()), this, SLOT(SendCustomMessage()));

  connect(m_Controls.buttonInitLaser, SIGNAL(clicked()), this, SLOT(InitLaser()));
  connect(m_Controls.buttonTune, SIGNAL(clicked()), this, SLOT(TuneWavelength()));
  connect(m_Controls.buttonFlashlamp, SIGNAL(clicked()), this, SLOT(ToggleFlashlamp()));
  connect(m_Controls.buttonQSwitch, SIGNAL(clicked()), this, SLOT(ToggleQSwitch()));

  connect(m_Controls.sliderWavelength, SIGNAL(valueChanged(int)), this, SLOT(SyncWavelengthSetBySlider()));
  connect(m_Controls.spinBoxWavelength, SIGNAL(valueChanged(double)), this, SLOT(SyncWavelengthSetBySpinBox()));

  m_SyncFromSpinBox = true;
  m_SyncFromSlider = true;

  m_LaserSystemConnected = false;
}
void OPOLaserControl::SyncWavelengthSetBySlider()
{
  if (m_SyncFromSlider)
  {
    m_SyncFromSpinBox = false;
    m_Controls.spinBoxWavelength->setValue(m_Controls.sliderWavelength->value() / 10);
  }
  else
    m_SyncFromSlider = true;
}

void OPOLaserControl::SyncWavelengthSetBySpinBox()
{
  if (m_SyncFromSpinBox)
  {
    m_SyncFromSlider = false;
    m_Controls.sliderWavelength->setValue(m_Controls.spinBoxWavelength->value() * 10);
  }
  else
    m_SyncFromSpinBox = true;
}

void OPOLaserControl::InitLaser()
{
  m_Controls.buttonInitLaser->setText("working ...");
  if (!m_LaserSystemConnected)
  {
    m_OpotekLaserSystem = mitk::OpotekLaser::New();
    m_OpotekLaserSystem->SetConfigurationPath("opotek_15033_SN3401_1905_1906_20160330.ini");
    m_OPOMotor = mitk::GalilMotor::New();
    m_OPOMotor->OpenConnection();

    if (m_OpotekLaserSystem->Initialize())
    {
      m_Controls.buttonFlashlamp->setEnabled(true);
      m_Controls.buttonQSwitch->setEnabled(true);
      m_Controls.buttonTune->setEnabled(true);
      m_Controls.buttonInitLaser->setText("Reset and Release Laser");

      m_Controls.sliderWavelength->setMinimum(m_OpotekLaserSystem->GetMinWavelength());
      m_Controls.sliderWavelength->setMaximum(m_OpotekLaserSystem->GetMaxWavelength());
      m_Controls.spinBoxWavelength->setMinimum(m_OpotekLaserSystem->GetMinWavelength() / 10.0);
      m_Controls.spinBoxWavelength->setMaximum(m_OpotekLaserSystem->GetMaxWavelength() / 10.0);
      m_Controls.sliderWavelength->setValue(m_OpotekLaserSystem->GetWavelength());
      m_Controls.spinBoxWavelength->setValue(m_OpotekLaserSystem->GetWavelength() / 10.0);
      m_LaserSystemConnected = true;
    }
    else
    {
      MITK_ERROR << "OpotekLaser Initialization failed.";
    }
  }
  else
  {
    // destroy and free
    if (m_OpotekLaserSystem->ResetAndRelease())
    {
      m_OPOMotor->CloseConnection();
      m_Controls.buttonFlashlamp->setEnabled(false);
      m_Controls.buttonQSwitch->setEnabled(false);
      m_Controls.buttonTune->setEnabled(false);
      m_Controls.buttonInitLaser->setText("Init Laser");
      m_LaserSystemConnected = false;
    }
    else
    {
      MITK_ERROR << "OpotekLaser release failed.";
    }
  }
}

void OPOLaserControl::TuneWavelength()
{
  m_OPOMotor->TuneToWavelength(30000/*m_Controls.spinBoxFIXME->value()*/);

  //QString wavelengthText = QString::number(m_OpotekLaserSystem->GetWavelength() / 10);
  //wavelengthText.append("nm");
  //m_Controls.labelWavelength->setText(wavelengthText);
}

void OPOLaserControl::ToggleFlashlamp()
{
  m_Controls.buttonFlashlamp->setText("...");
  if (!m_OpotekLaserSystem->IsFlashing())
  {
    m_OpotekLaserSystem->StartFlashing();
    m_Controls.buttonFlashlamp->setText("Stop Lamp");
  }
  else
  {
    m_OpotekLaserSystem->StopFlashing();
    m_Controls.buttonFlashlamp->setText("Start Lamp");
  }
}

void OPOLaserControl::ToggleQSwitch()
{
  m_Controls.buttonQSwitch->setText("...");
  if (!m_OpotekLaserSystem->IsEmitting())
  {
    m_OpotekLaserSystem->StartQswitching();
    m_Controls.buttonQSwitch->setText("Stop Laser");
  }
  else
  {
    m_OpotekLaserSystem->StopQswitching();
    m_Controls.buttonQSwitch->setText("Start Laser");
  }
}

void OPOLaserControl::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
  const QList<mitk::DataNode::Pointer>& nodes)
{
}

void OPOLaserControl::ConnectToLaser()
{
  m_PumpLaserController = mitk::OpotekPumpLaserController::New();
  if (m_PumpLaserController->OpenConnection())
  {
    m_Controls.buttonSendCustomMessage->setEnabled(true);
    m_Controls.buttonStatus->setEnabled(true);
    m_Controls.buttonConnect->setText("Disconnect");
    std::string message("TRIG EE"); // both external Triggers
    std::string response("");

    m_PumpLaserController->Send(&message);
    m_PumpLaserController->ReceiveLine(&response);

    ////get port
    //int port = 0;
    //port = m_Controls.spinBoxPort->value();

    ////build prefix (depends on linux/win)
    //QString prefix = "";
    //#ifdef WIN32
    //  prefix = "COM";
    //  m_serial->SetPortNumber(static_cast<mitk::SerialCommunication::PortNumber>(port)); //also set the com port for compatibility
    //#else
    //  prefix = m_Controls.comboBoxPortType->currentText();
    //#endif

    //QString portName = prefix + QString::number(port);
    //m_serial->SetDeviceName(portName.toStdString());

    // FIXME Unclear specs
    //  • Half duplex
    //  • Does not use Xon/Xoff
    //  • Does not use RTS/CTS
    // FIXME
  }
  else
  {
    m_PumpLaserController->CloseConnection();
    m_Controls.buttonSendCustomMessage->setEnabled(false);
    m_Controls.buttonStatus->setEnabled(false);
    m_Controls.buttonConnect->setText("Connect");
  }
}

void OPOLaserControl::GetStatus()
{
  mitk::OpotekPumpLaserController::PumpLaserState pumpLaserState = m_PumpLaserController->GetState();

  if (pumpLaserState == mitk::OpotekPumpLaserController::STATE0)
    MITK_INFO << "Received STATE0: Boot Fault.";
  else if (pumpLaserState == mitk::OpotekPumpLaserController::STATE1)
    MITK_INFO << "Received STATE1: Warm Up.";
  else if (pumpLaserState == mitk::OpotekPumpLaserController::STATE2)
    MITK_INFO << "Received STATE2: Laser Ready.";
  else if (pumpLaserState == mitk::OpotekPumpLaserController::STATE3)
    MITK_INFO << "Received STATE3: Flashing. Pulse Disabled.";
  else if (pumpLaserState == mitk::OpotekPumpLaserController::STATE4)
    MITK_INFO << "Received STATE4: Flashing. Shutter Closed.";
  else if (pumpLaserState == mitk::OpotekPumpLaserController::STATE5)
    MITK_INFO << "Received STATE5: Flashing. Pulse Enabled.";
  else if (pumpLaserState == mitk::OpotekPumpLaserController::UNCONNECTED)
    MITK_INFO << "Received ERROR.";
}

void OPOLaserControl::SendCustomMessage()
{
  std::string message = m_Controls.lineMessage->text().toStdString();
  std::string response("");

  m_PumpLaserController->Send(&message);
  m_PumpLaserController->ReceiveLine(&response);

  MITK_INFO << "Received response: " << response;
}

void OPOLaserControl::ToogleFlashlamp()
{
  m_PumpLaserController->StartFlashlamps();
  MITK_INFO << "Received response: ";
}