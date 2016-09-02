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
#include <mitkPumpLaserController.h>

const std::string OPOLaserControl::VIEW_ID = "org.mitk.views.opolasercontrol";

void OPOLaserControl::SetFocus()
{
  m_Controls.buttonConnect->setFocus();
}

void OPOLaserControl::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  connect( m_Controls.buttonConnect, SIGNAL(clicked()), this, SLOT(ConnectToLaser()) );
  connect( m_Controls.buttonStatus, SIGNAL(clicked()), this, SLOT(GetStatus()) );
  connect( m_Controls.buttonSendCustomMessage, SIGNAL(clicked()), this, SLOT(SendCustomMessage()) );
}

void OPOLaserControl::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                             const QList<mitk::DataNode::Pointer>& nodes )
{

}


void OPOLaserControl::ConnectToLaser()
{
  mitk::PumpLaserController * test = mitk::PumpLaserController::New;
  MITK_INFO << "!!!!!!!!!!-??-------!!!!!!!!!!!!!";
  if (m_PumpLaserController->OpenConnection())
  {
    m_Controls.buttonSendCustomMessage->setEnabled(true);
    m_Controls.buttonStatus->setEnabled(true);
    m_Controls.buttonConnect->setText("Disconnect");
    MITK_INFO << "!!!!!!!!!!----------!!!!!!!!!!!!!";
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
  mitk::PumpLaserController::PumpLaserState pumpLaserState = m_PumpLaserController->GetState();

  if (pumpLaserState == mitk::PumpLaserController::STATE0)
    MITK_INFO << "Received STATE0: Boot Fault.";
  else if (pumpLaserState == mitk::PumpLaserController::STATE1)
    MITK_INFO << "Received STATE1: Warm Up.";
  else if (pumpLaserState == mitk::PumpLaserController::STATE2)
    MITK_INFO << "Received STATE2: Laser Ready.";
  else if (pumpLaserState == mitk::PumpLaserController::STATE3)
    MITK_INFO << "Received STATE3: Flashing. Pulse Disabled.";
  else if (pumpLaserState == mitk::PumpLaserController::STATE4)
    MITK_INFO << "Received STATE4: Flashing. Shutter Closed.";
  else if (pumpLaserState == mitk::PumpLaserController::STATE5)
    MITK_INFO << "Received STATE5: Flashing. Pulse Enabled.";
  else if (pumpLaserState == mitk::PumpLaserController::UNCONNECTED)
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