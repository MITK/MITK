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
#include "OpenIGTLink.h"

// Qt
#include <QMessageBox>

//mitk image
#include <mitkImage.h>

const std::string OpenIGTLink::VIEW_ID = "org.mitk.views.openigtlink";

void OpenIGTLink::SetFocus()
{
//  m_Controls.buttonPerformImageProcessing->setFocus();
}

void OpenIGTLink::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  // connect the widget items with the methods
  connect( m_Controls.butConnectWithServer, SIGNAL(clicked()),
           this, SLOT(ConnectWithServer()));
  connect( m_Controls.editPort, SIGNAL(editingFinished()),
           this, SLOT(ChangePort()) );
  connect( m_Controls.editIP, SIGNAL(editingFinished()),
           this, SLOT(ChangeIP()) );

  // set the validator for the ip edit box (values must be between 0 and 255 and
  // there are four of them, seperated with a point
  QRegExpValidator *v = new QRegExpValidator(this);
  QRegExp rx("((1{0,1}[0-9]{0,2}|2[0-4]{1,1}[0-9]{1,1}|25[0-5]{1,1})\\.){3,3}(1{0,1}[0-9]{0,2}|2[0-4]{1,1}[0-9]{1,1}|25[0-5]{1,1})");
  v->setRegExp(rx);
  m_Controls.editIP->setValidator(v);
  // set the validator for the port edit box (values must be between 1 and 65535)
  m_Controls.editPort->setValidator(new QIntValidator(1, 65535, this));

  m_IGTLClient = mitk::IGTLClient::New();
}

void OpenIGTLink::ConnectWithServer()
{
    if(m_Controls.butConnectWithServer->text() == "Connect")
    {
        m_Controls.editIP->setEnabled(false);
        m_Controls.editPort->setEnabled(false);
        m_Controls.butConnectWithServer->setText("Disconnect");

        m_IGTLClient->SetPortNumber(m_Controls.editPort->text().toInt());
        m_IGTLClient->SetHostname(m_Controls.editIP->text().toStdString());
        m_IGTLClient->OpenConnection();
    }
    else
    {
        m_Controls.editIP->setEnabled(true);
        m_Controls.editPort->setEnabled(true);
        m_Controls.butConnectWithServer->setText("Connect");
        m_IGTLClient->CloseConnection();
    }
}

void OpenIGTLink::ChangePort()
{

}

void OpenIGTLink::ChangeIP()
{

}
