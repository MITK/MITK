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

// Qmitk
#include "QmitkToFConnectView.h"

// Qt
#include <QString>
#include <QStringList>

#include <usServiceReference.h>
#include <mitkGetModuleContext.h>
#include <mitkIToFDeviceFactory.h>
#include <mitkToFDeviceFactoryManager.h>
#include <mitkToFCameraDevice.h>

#include <QmitkServiceListWidget.h>



//Microservices
#include <usGetModuleContext.h>
#include <usModule.h>
//#include <usServiceProperties.h>
#include "mitkModuleContext.h"
#include <usServiceInterface.h>
#include <usServiceRegistration.h>


const std::string QmitkToFConnectView::VIEW_ID = "org.mitk.views.tofconnectview";

QmitkToFConnectView::QmitkToFConnectView()
    : QmitkAbstractView()
{
}

QmitkToFConnectView::~QmitkToFConnectView()
{
}

void QmitkToFConnectView::SetFocus()
{
//    this->m_Controls.m_CameraList->setFocus();
//    GetRegisteredDeviceFactories();
}

void QmitkToFConnectView::CreateQtPartControl( QWidget *parent )
{
        // create GUI widgets from the Qt Designer's .ui file
        m_Controls.setupUi( parent );

        //GetRegistered-Button
        connect( (QObject*)(m_Controls.m_GetRegistered), SIGNAL(clicked()), this, SLOT(GetRegisteredDeviceFactories()) );
        //GetConnect-Button
        connect( (QObject*)(m_Controls.m_GetConnect), SIGNAL(clicked()), this, SLOT(GetConnectedDevices()) );

        //TODO: Connect Device-Button
        //connect( (QObject*)(m_Controls->m_ConnectDeive), SIGNAL(clicked()), this, SLOT(OnToFCameraConnected()) );


}


void QmitkToFConnectView::GetRegisteredDeviceFactories()
{
    std::string empty= "";
    m_Controls.m_DeviceFactoryServiceListWidget->Initialize<mitk::IToFDeviceFactory>("ToFFactoryName", empty);
}


void QmitkToFConnectView::GetConnectedDevices()
{
    std::string empty= "";
    m_Controls.m_ConnectedDeviceServiceListWidget->Initialize<mitk::ToFCameraDevice>("ToFFactoryName", empty);
}

/* TODO: Method to Connect Camera
void QmitkToFConnectView::OnToFCameraConnected()
{
}
*/
