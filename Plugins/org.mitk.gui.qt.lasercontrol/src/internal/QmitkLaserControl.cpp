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
#include <QtConcurrentRun>

const std::string OPOLaserControl::VIEW_ID = "org.mitk.views.lasercontrol";

void OPOLaserControl::SetFocus()
{
}

void OPOLaserControl::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  connect(m_Controls.buttonInitLaser, SIGNAL(clicked()), this, SLOT(InitResetLaser()));
  connect(m_Controls.buttonTune, SIGNAL(clicked()), this, SLOT(TuneWavelength()));
  connect(m_Controls.buttonFastTuning, SIGNAL(clicked()), this, SLOT(StartFastTuning()));
  connect(m_Controls.buttonFlashlamp, SIGNAL(clicked()), this, SLOT(ToggleFlashlamp()));
  connect(m_Controls.buttonQSwitch, SIGNAL(clicked()), this, SLOT(ToggleQSwitch()));
  connect(m_Controls.sliderWavelength, SIGNAL(valueChanged(int)), this, SLOT(SyncWavelengthSetBySlider()));
  connect(m_Controls.spinBoxWavelength, SIGNAL(valueChanged(double)), this, SLOT(SyncWavelengthSetBySpinBox()));
  connect(&m_ShutterWatcher, SIGNAL(finished()), this, SLOT(EnableLaser()));

  m_SyncFromSpinBox = true;
  m_SyncFromSlider = true;

  m_PumpLaserConnected = false;
  m_OPOConnected = false;
  m_PyroConnected = false;
}

void OPOLaserControl::EnableLaser()
{
  m_Controls.buttonQSwitch->setEnabled(true);
  m_Controls.buttonQSwitch->setText("Start Laser");
  this->GetState();
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

void OPOLaserControl::InitResetLaser()
{
  m_Controls.buttonInitLaser->setEnabled(false);
  m_Controls.buttonInitLaser->setText("working ...");

  if (!m_PumpLaserConnected && !m_OPOConnected)
  {
    m_PumpLaserController = mitk::QuantelLaser::New();
    m_OPOMotor = mitk::GalilMotor::New();
    InitThread *initThread = new InitThread(m_PumpLaserController, m_OPOMotor);
    connect(initThread, SIGNAL(result(bool, bool)), this, SLOT(InitLaser(bool, bool)));
    connect(initThread, SIGNAL(finished()), initThread, SLOT(deleteLater()));
    initThread->start();
  }
  else
  {
    // destroy and free
    m_Controls.buttonFlashlamp->setEnabled(false);
    m_Controls.buttonQSwitch->setEnabled(false);
    m_Controls.buttonTune->setEnabled(false);
    m_Controls.buttonFastTuning->setEnabled(false);

    ResetThread *resetThread = new ResetThread(m_PumpLaserController, m_OPOMotor);
    connect(resetThread, SIGNAL(result(bool, bool)), this, SLOT(ResetLaser(bool, bool)));
    connect(resetThread, SIGNAL(finished()), resetThread, SLOT(deleteLater()));
    resetThread->start();
  }
  /*
  try
  {
    if (!m_PyroConnected)
    {
      m_Pyro = mitk::OphirPyro::New();
      MITK_INFO << "[Pyro Debug] OpenConnection: " << m_Pyro->OpenConnection();
      MITK_INFO << "[Pyro Debug] StartDataAcquisition: " << m_Pyro->StartDataAcquisition();
      m_CurrentPulseEnergy = 0;
      m_PyroConnected = true;
      //QFuture<void> future = QtConcurrent::run(this, &OPOLaserControl::ShowEnergy);
      //m_EnergyWatcher.setFuture(future);
    }
    else
    {
      m_PyroConnected = false;
    }
  } catch (...) {
    MITK_INFO << " While trying to connect to the Pyro an exception was caught (this almost always happens on the first try after reboot - try again!)";
    m_PyroConnected = false;
  }*/
}

void OPOLaserControl::InitLaser(bool successLaser, bool successMotor)
{
  if (successLaser && successMotor)
  {
    m_Controls.buttonFlashlamp->setEnabled(true);
    m_Controls.buttonQSwitch->setEnabled(false);
    m_Controls.buttonInitLaser->setText("Reset and Release Laser");

    std::string answer("");
    std::string triggerCommand("TRIG ");
    if (m_Controls.checkBoxTriggerExternally->isChecked())
    {
      triggerCommand.append("EE"); // set both Triggers external
      m_PumpLaserController->SendAndReceiveLine(&triggerCommand, &answer);
      MITK_INFO << answer;
    }
    else
    {
      triggerCommand.append("II"); // set both Triggers internal
      m_PumpLaserController->SendAndReceiveLine(&triggerCommand, &answer);
      MITK_INFO << answer;
      std::string energyCommand("QDLY 30");
      m_PumpLaserController->SendAndReceiveLine(&energyCommand, &answer);
      MITK_INFO << answer;
    }

    m_PumpLaserConnected = true;

    m_Controls.buttonTune->setEnabled(true);
    m_Controls.buttonFastTuning->setEnabled(true);
    m_Controls.sliderWavelength->setMinimum(m_OPOMotor->GetMinWavelength() * 10);
    m_Controls.sliderWavelength->setMaximum(m_OPOMotor->GetMaxWavelength() * 10);
    m_Controls.spinBoxWavelength->setMinimum(m_OPOMotor->GetMinWavelength());
    m_Controls.spinBoxWavelength->setMaximum(m_OPOMotor->GetMaxWavelength());
    m_Controls.sliderWavelength->setValue(m_OPOMotor->GetCurrentWavelength() * 10);
    m_Controls.spinBoxWavelength->setValue(m_OPOMotor->GetCurrentWavelength());

    m_OPOConnected = true; // not always right FIXME

    this->GetState();
  }
  else
  {
    if(!successLaser)
      QMessageBox::warning(NULL, "Laser Control", "Opotek Pump Laser Initialization Failed.");
    if(!successMotor)
      QMessageBox::warning(NULL, "Laser Control", "OPO Initialization Failed.");

    m_Controls.buttonInitLaser->setText("Init Laser");
    return;
  }

  m_Controls.buttonInitLaser->setEnabled(true);
  this->GetState();
}

void OPOLaserControl::ResetLaser(bool successLaser, bool successMotor)
{
  if (successLaser && successMotor)
  {
    m_Controls.buttonFlashlamp->setEnabled(false);
    m_Controls.buttonQSwitch->setEnabled(false);
    m_Controls.buttonInitLaser->setText("Init Laser");
    m_PumpLaserConnected = false;

    m_Controls.buttonTune->setEnabled(false);
    m_Controls.buttonFastTuning->setEnabled(false);
    m_Controls.buttonInitLaser->setText("Init Laser");
    m_OPOConnected = false;
  }
  else
  {
    if(!successMotor)
      QMessageBox::warning(NULL, "Laser Control", "OPO Release Failed.");
    if(!successLaser)
      QMessageBox::warning(NULL, "Laser Control", "Opotek Pump Laser Release Failed.");

    m_Controls.buttonInitLaser->setText("Reset and Release Laser");
  }

  m_Controls.buttonInitLaser->setEnabled(true);
  this->GetState();
}

void OPOLaserControl::TuneWavelength()
{
  if (m_Controls.checkBoxCalibration->isChecked())
  {
    m_OPOMotor->TuneToWavelength(m_Controls.spinBoxPosition->value(), true);
  }
  else
  {
    m_OPOMotor->TuneToWavelength(m_Controls.spinBoxWavelength->value(), false);
  }
  QString wavelengthText = QString::number(m_OPOMotor->GetCurrentWavelength());
  wavelengthText.append("nm");
  m_Controls.labelWavelength->setText(wavelengthText);
}

void OPOLaserControl::StartFastTuning()
{
  std::vector<double> listOfWavelengths;
  double tmpWavelength = 0;
  int currentRow = 0;
  bool success = false;

  do
  {
    if (currentRow != 0) listOfWavelengths.push_back(tmpWavelength);
    if (m_Controls.tableFastTuningWavelengths->item(0, currentRow))
    {
      QString test = m_Controls.tableFastTuningWavelengths->item(0, currentRow)->text();
      tmpWavelength = test.toDouble(&success);
      currentRow++;
    }
    else
      tmpWavelength = 0;

    if (success == 0)
      tmpWavelength = 0;
  } while (tmpWavelength<950.1 && tmpWavelength>689.9);

  m_OPOMotor->FastTuneWavelengths(listOfWavelengths);
}

void OPOLaserControl::ShutterCountDown()
{
  m_Controls.buttonFlashlamp->setText("Stop Lamp");
  m_Controls.buttonQSwitch->setEnabled(false);
  m_Controls.buttonQSwitch->setText("10s ...");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  m_Controls.buttonQSwitch->setText("9s ...");
  std::this_thread::sleep_for(std::chrono::seconds(1));  
  m_Controls.buttonQSwitch->setText("8s ...");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  m_Controls.buttonQSwitch->setText("7s ...");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  m_Controls.buttonQSwitch->setText("6s ...");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  m_Controls.buttonQSwitch->setText("5s ...");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  m_Controls.buttonQSwitch->setText("4s ...");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  m_Controls.buttonQSwitch->setText("3s ...");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  m_Controls.buttonQSwitch->setText("2s ...");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  m_Controls.buttonQSwitch->setText("1s ...");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return;
}

void OPOLaserControl::ShowEnergy()
{
  /*forever
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    MITK_INFO << "[Pyro Debug] ShowEnergy()";
    if (!m_PyroConnected)
    {
      MITK_INFO << "[Pyro Debug] StopDataAcquisition: " << m_Pyro->StopDataAcquisition();
      MITK_INFO << "[Pyro Debug] CloseConnection: " << m_Pyro->CloseConnection();
      m_CurrentPulseEnergy = 0;
      return;
    }
  //  m_Pyro->GetDataFromSensor();
    m_CurrentPulseEnergy = 60000 * m_Pyro->LookupCurrentPulseEnergy();
    m_Controls.labelEnergy->setText(std::to_string(m_CurrentPulseEnergy).append(" mJ").c_str());
  }*/
}

void OPOLaserControl::ToggleFlashlamp()
{
  m_Controls.buttonFlashlamp->setText("...");
  if (!m_PumpLaserController->IsFlashing())
  {
    if (m_PumpLaserController->StartFlashing())
    {
      QFuture<void> future = QtConcurrent::run(this, &OPOLaserControl::ShutterCountDown);
      m_ShutterWatcher.setFuture(future);
    }
    else
      m_Controls.buttonFlashlamp->setText("Start Lamp");
  }
  else
  {
    if (m_PumpLaserController->StopFlashing())
    {
      m_Controls.buttonFlashlamp->setText("Start Lamp");
      m_Controls.buttonQSwitch->setText("Start Laser");
      m_Controls.buttonQSwitch->setEnabled(false);
    }
    else
      m_Controls.buttonFlashlamp->setText("Stop Lamp");
  }
  this->GetState();
}

void OPOLaserControl::ToggleQSwitch()
{
  m_Controls.buttonQSwitch->setText("...");
  if (!m_PumpLaserController->IsEmitting())
  {
    if (m_PumpLaserController->StartQswitching())
      m_Controls.buttonQSwitch->setText("Stop Laser");
    else
      m_Controls.buttonQSwitch->setText("Start Laser");
  }
  else
  {
    if (m_PumpLaserController->StopQswitching())
      m_Controls.buttonQSwitch->setText("Start Laser");
    else
      m_Controls.buttonQSwitch->setText("Stop Laser");
  }
  this->GetState();
}

void OPOLaserControl::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
  const QList<mitk::DataNode::Pointer>& nodes)
{
}

void OPOLaserControl::GetState()
{
  mitk::QuantelLaser::LaserState pumpLaserState = m_PumpLaserController->GetState();

  if (pumpLaserState == mitk::QuantelLaser::STATE0)
    m_Controls.labelStatus->setText("PL0: Boot Fault.");
  else if (pumpLaserState == mitk::QuantelLaser::STATE1)
    m_Controls.labelStatus->setText("PL1: Warm Up.");
  else if (pumpLaserState == mitk::QuantelLaser::STATE2)
    m_Controls.labelStatus->setText("PL2: Laser Ready.");
  else if (pumpLaserState == mitk::QuantelLaser::STATE3)
    m_Controls.labelStatus->setText("PL3: Flashing. Pulse Disabled.");
  else if (pumpLaserState == mitk::QuantelLaser::STATE4)
    m_Controls.labelStatus->setText("PL4: Flashing. Shutter Closed.");
  else if (pumpLaserState == mitk::QuantelLaser::STATE5)
    m_Controls.labelStatus->setText("PL5: Flashing. Shutter Open.");
  else if (pumpLaserState == mitk::QuantelLaser::STATE6)
    m_Controls.labelStatus->setText("PL6: Flashing. Pulse Enabled.");
  else if (pumpLaserState == mitk::QuantelLaser::STATE7)
    m_Controls.labelStatus->setText("PL7: Undefined State.");
  else if (pumpLaserState == mitk::QuantelLaser::UNCONNECTED)
    m_Controls.labelStatus->setText("PL Not Connected.");
}