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

//mitk image
#include <mitkImage.h>


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
  m_serial = mitk::SerialCommunication::New();
}

void OPOLaserControl::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                             const QList<mitk::DataNode::Pointer>& nodes )
{

}


void OPOLaserControl::ConnectToLaser()
{
  MITK_INFO << m_serial->IsConnected();
  if(m_serial->IsConnected())
  {
    m_serial->CloseConnection();
    m_Controls.buttonSendCustomMessage->setEnabled(false);
    m_Controls.buttonStatus->setEnabled(false);
    m_Controls.buttonConnect->setText("Connect");
  }

  else
  {
    m_serial->SetBaudRate(mitk::SerialCommunication::BaudRate115200);

    //get port
    int port = 0;
    port = m_Controls.spinBoxPort->value();

    //build prefix (depends on linux/win)
    QString prefix = "";
    #ifdef WIN32
      prefix = "COM";
      m_serial->SetPortNumber(static_cast<mitk::SerialCommunication::PortNumber>(port)); //also set the com port for compatibility
    #else
      prefix = m_Controls.comboBoxPortType->currentText();
    #endif

    QString portName = prefix + QString::number(port);
    m_serial->SetDeviceName(portName.toStdString());

    m_serial->SetDataBits(mitk::SerialCommunication::DataBits8);
    m_serial->SetParity(mitk::SerialCommunication::None);  // No parity
    m_serial->SetStopBits(mitk::SerialCommunication::StopBits1);

    // FIXME Unclear specs
    //  • Half duplex
    //  • Does not use Xon/Xoff
    //  • Does not use RTS/CTS
    // FIXME

    m_serial->SetHardwareHandshake(mitk::SerialCommunication::HardwareHandshakeOff);
    m_serial->SetSendTimeout(200);
    m_serial->SetReceiveTimeout(200);

    MITK_INFO << "sent STATE Command";
    m_serial->OpenConnection();
    m_Controls.buttonSendCustomMessage->setEnabled(true);
    m_Controls.buttonStatus->setEnabled(true);
    m_Controls.buttonConnect->setText("Disconnect");
  }
}

void OPOLaserControl::GetStatus()
{
  std::string message = "STATE";
  std::string tmpMessage = "";

  m_serial->Send(message);

  if (m_serial->Receive(tmpMessage,4) == false) // receive 4 bytes
    std::cout << "Error receiving STATE. Only " << tmpMessage.size() << " characters received: '" << tmpMessage << "'.\n";
  else
    std::cout << "Received STATE: '" << tmpMessage << "'.\n";

  //  COMMAND SYNTAX
  //  • Commands are in ASCII, UPPER CASE only, and must be terminated by the “CR” or “LF” or “CRLF” (Carriage Return Line Feed) character.
  //  • Responses are messages followed by a LF.
  //  • Commands can be sent at a maximum rate of 10 Hz. Sending a new command only after receiving the response from the previous one is the best way to ensure you meet this requirement.
  //  • Answer message ‘ERROR: UNKNOWN’ is returned when the computer doesn’t recognize the characters sent,
  //  • Answer message ‘ERROR : OUT OF RANGE’ is returned when, while programming a data, the number of figure exceeds the authorized one,
  //  • Answer message ‘ERROR : BAD PARAM’ is returned when the entered character is not an expected numeric character.

}

void OPOLaserControl::SendCustomMessage()
{
  std::string message = m_Controls.lineMessage->text().toStdString();
  std::string response = "";

  m_serial->Send(message);

  if (m_serial->Receive(response,4) == false) // receive 4 bytes
    MITK_INFO << "Error receiving response. Only " << response.size() << " characters received: '" << response;
  else
    MITK_INFO << "Received response: '" << response;

}
