/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "OpenIGTLinkPlugin.h"

// Qt
#include <QMessageBox>

//mitk image
#include <mitkImage.h>

//VTK
#include <vtkSmartPointer.h>

const std::string OpenIGTLinkPlugin::VIEW_ID = "org.mitk.views.openigtlinkplugin";

void OpenIGTLinkPlugin::SetFocus()
{
}

void OpenIGTLinkPlugin::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  connect(m_Controls.buttonConnect, SIGNAL(clicked()), this, SLOT(ConnectButtonClicked()));
  connect(m_Controls.buttonReceive, SIGNAL(clicked()), this, SLOT(ReceivingButtonClicked()));
  connect(&m_Timer, SIGNAL(timeout()), this, SLOT(UpdatePipeline()));

  m_Image2dNode = mitk::DataNode::New();

  m_State = IDLE;
  StateChanged(m_State);
}

void OpenIGTLinkPlugin::UpdatePipeline()
{
  m_NavigationDataObjectVisualizationFilter->Update();
  mitk::Image::Pointer image2d = m_ImageFilter2D->GetNextImage().at(0);
  mitk::Image::Pointer image3d = m_ImageFilter3D->GetNextImage().at(0);

  m_Image2dNode->SetName("US Image Stream");
  m_Image2dNode->SetData(image2d);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void OpenIGTLinkPlugin::OnSelectionChanged(berry::IWorkbenchPart::Pointer,
  const QList<mitk::DataNode::Pointer> &)
{
  // iterate all selected objects, adjust warning visibility
}

void OpenIGTLinkPlugin::ConnectButtonClicked()
{
  bool success;

  switch (m_State)
  {
  case IDLE:
    m_IGTLClient = mitk::IGTLClient::New(true);
    m_IGTLClient->SetHostname(m_Controls.textEditHostname->text().toStdString());
    m_IGTLClient->SetPortNumber(m_Controls.spinBoxPort->value());

    success = m_IGTLClient->OpenConnection();

    if (!success)
    {
      QMessageBox::warning(nullptr, QString("Connection failed"), QString("Client could not connect to given server."),
        QMessageBox::Ok, QMessageBox::Abort);
    }
    else
    {
      m_State = CONNECTED;
      StateChanged(m_State);
    }
    break;
  case CONNECTED:
    success = m_IGTLClient->CloseConnection();
    m_State = IDLE;
    StateChanged(m_State);
    break;
  case RECEIVING:
    ReceivingButtonClicked();
    success = m_IGTLClient->CloseConnection();
    m_State = IDLE;
    StateChanged(m_State);
    break;
  }
}

void OpenIGTLinkPlugin::ReceivingButtonClicked()
{
  switch (m_State)
  {
  case IDLE:
    QMessageBox::warning(nullptr, QString("Not ready.."), QString("The client must be connected to a server first."),
      QMessageBox::Ok, QMessageBox::Abort);
    break;
  case CONNECTED:
    m_IGTL2DImageDeviceSource = mitk::IGTL2DImageDeviceSource::New();
    m_IGTL3DImageDeviceSource = mitk::IGTL3DImageDeviceSource::New();
    m_IGTLTransformDeviceSource = mitk::IGTLTrackingDataDeviceSource::New();

    m_IGTL2DImageDeviceSource->SetIGTLDevice(m_IGTLClient);
    m_IGTL3DImageDeviceSource->SetIGTLDevice(m_IGTLClient);
    m_IGTLTransformDeviceSource->SetIGTLDevice(m_IGTLClient);

    this->GetDataStorage()->Add(m_Image2dNode);

    m_IGTLMessageToNavigationDataFilter = mitk::IGTLMessageToNavigationDataFilter::New();
    m_NavigationDataObjectVisualizationFilter = mitk::NavigationDataObjectVisualizationFilter::New();
    m_ImageFilter2D = mitk::IGTLMessageToUSImageFilter::New();
    m_ImageFilter3D = mitk::IGTLMessageToUSImageFilter::New();

    m_IGTLMessageToNavigationDataFilter->SetNumberOfExpectedOutputs(3);

    m_IGTLMessageToNavigationDataFilter->ConnectTo(m_IGTLTransformDeviceSource);
    m_NavigationDataObjectVisualizationFilter->ConnectTo(m_IGTLMessageToNavigationDataFilter);

    m_ImageFilter2D->ConnectTo(m_IGTL2DImageDeviceSource);
    m_ImageFilter3D->ConnectTo(m_IGTL3DImageDeviceSource);

    //create an object that will be moved respectively to the navigation data
    for (size_t i = 0; i < m_IGTLMessageToNavigationDataFilter->GetNumberOfIndexedOutputs(); i++)
    {
      mitk::DataNode::Pointer newNode = mitk::DataNode::New();
      QString name("DemoNode T");
      name.append(QString::number(i));
      newNode->SetName(name.toStdString());

      //create small sphere and use it as surface
      mitk::Surface::Pointer mySphere = mitk::Surface::New();
      vtkSmartPointer<vtkSphereSource> vtkSphere = vtkSmartPointer<vtkSphereSource>::New();
      vtkSphere->SetRadius(2.0f);
      vtkSphere->SetCenter(0.0, 0.0, 0.0);
      vtkSphere->Update();
      mySphere->SetProperty("color", mitk::ColorProperty::New(1, 0, 0));
      mySphere->SetVtkPolyData(vtkSphere->GetOutput());
      newNode->SetData(mySphere);

      this->GetDataStorage()->Add(newNode);

      m_NavigationDataObjectVisualizationFilter->SetRepresentationObject(i, mySphere.GetPointer());
    }

    m_IGTLClient->StartCommunication();
    m_Timer.setInterval(10);
    m_Timer.start();

    m_State = RECEIVING;
    StateChanged(m_State);
    break;
  case RECEIVING:
    m_IGTLClient->StopCommunication();
    this->GetDataStorage()->Remove(this->GetDataStorage()->GetAll());
    m_Timer.stop();
    m_State = CONNECTED;
    StateChanged(m_State);
    break;
  }
}

void OpenIGTLinkPlugin::StateChanged(OpenIGTLinkPlugin::State newState)
{
  switch (newState)
  {
  case IDLE:
    m_Controls.buttonConnect->setText(QString("Connect To Server"));
    m_Controls.buttonReceive->setText(QString("Start Receiving"));
    m_Controls.buttonReceive->setDisabled(true);
    break;
  case CONNECTED:
    m_Controls.buttonConnect->setText(QString("Disconnect From Server"));
    m_Controls.buttonReceive->setText(QString("Start Receiving"));
    m_Controls.buttonReceive->setDisabled(false);
    break;
  case RECEIVING:
    m_Controls.buttonConnect->setText(QString("Disconnect From Server"));
    m_Controls.buttonReceive->setText(QString("Stop Receiving"));
    m_Controls.buttonReceive->setDisabled(false);
    break;
  }
}
