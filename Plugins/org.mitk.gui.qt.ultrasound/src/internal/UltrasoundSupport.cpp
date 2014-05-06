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

//Mitk
#include <mitkDataNode.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkIRenderingManager.h>

// Qmitk
#include "UltrasoundSupport.h"
#include <QTimer>

// Qt
#include <QMessageBox>

// Ultrasound
#include "mitkUSDevice.h"
#include "QmitkUSAbstractCustomWidget.h"

#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include "usServiceReference.h"
#include "internal/org_mitk_gui_qt_ultrasound_Activator.h"

const std::string UltrasoundSupport::VIEW_ID = "org.mitk.views.ultrasoundsupport";

void UltrasoundSupport::SetFocus()
{
}

void UltrasoundSupport::CreateQtPartControl( QWidget *parent )
{
  m_Timer = new QTimer(this);

  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  connect( m_Controls.m_DeviceManagerWidget, SIGNAL(NewDeviceButtonClicked()), this, SLOT(OnClickedAddNewDevice()) ); // Change Widget Visibilities
  connect( m_Controls.m_DeviceManagerWidget, SIGNAL(NewDeviceButtonClicked()), this->m_Controls.m_NewVideoDeviceWidget, SLOT(CreateNewDevice()) ); // Init NewDeviceWidget
  connect( m_Controls.m_NewVideoDeviceWidget, SIGNAL(Finished()), this, SLOT(OnNewDeviceWidgetDone()) ); // After NewDeviceWidget finished editing
  connect( m_Controls.m_BtnView, SIGNAL(clicked()), this, SLOT(OnClickedViewDevice()) );
  connect( m_Controls.m_FrameRate, SIGNAL(valueChanged(int)), this, SLOT(OnChangedFramerateLimit(int)) );
  connect( m_Controls.m_FreezeButton, SIGNAL(clicked()), this, SLOT(OnClickedFreezeButton()) );
  connect( m_Timer, SIGNAL(timeout()), this, SLOT(DisplayImage()));

  // Initializations
  m_Controls.m_NewVideoDeviceWidget->setVisible(false);
  std::string filter = "(&(" + us::ServiceConstants::OBJECTCLASS() + "="
      + "org.mitk.services.UltrasoundDevice)("
      + mitk::USDevice::GetPropertyKeys().US_PROPKEY_ISACTIVE + "=true))";
  m_Controls.m_ActiveVideoDevices->Initialize<mitk::USDevice>(
        mitk::USDevice::GetPropertyKeys().US_PROPKEY_LABEL ,filter);

  m_Node = this->GetDataStorage()->GetNamedNode("US Image Stream");
  if (m_Node.IsNull())
  {
    // Create Node for US Stream
    m_Node = mitk::DataNode::New();
    m_Node->SetName("US Image Stream");
    this->GetDataStorage()->Add(m_Node);
  }

  m_Controls.tabWidget->setTabEnabled(1, false);
}

void UltrasoundSupport::OnClickedAddNewDevice()
{
  m_Controls.m_NewVideoDeviceWidget->setVisible(true);
  m_Controls.m_DeviceManagerWidget->setVisible(false);
  m_Controls.m_Headline->setText("Add New Video Device:");
  m_Controls.m_WidgetActiveDevices->setVisible(false);
}

void UltrasoundSupport::DisplayImage()
{
  m_Device->Modified();
  m_Device->Update();

  if ( m_ImageAlreadySetToNode && curOutput.GetPointer() != m_Node->GetData() )
  {
    MITK_INFO << "Data Node of the ultrasound image stream was changed by another plugin. Stop viewing.";
    this->StopViewing();
    return;
  }

  mitk::Image::Pointer curOutput = m_Device->GetOutput();
  if (! m_ImageAlreadySetToNode && curOutput.IsNotNull() && curOutput->IsInitialized())
  {
    m_Node->SetData(curOutput);
    m_ImageAlreadySetToNode = true;
  }

  this->RequestRenderWindowUpdate();

  if ( curOutput->GetDimension() > 1
    && (curOutput->GetDimension(0) != m_CurrentImageWidth
    || curOutput->GetDimension(1) != m_CurrentImageHeight) )
  {
    // make a reinit on the ultrasound image
    mitk::IRenderWindowPart* renderWindow = this->GetRenderWindowPart();
    if ( renderWindow != NULL && curOutput->GetTimeGeometry()->IsValid() )
    {
      renderWindow->GetRenderingManager()->InitializeViews(
        curOutput->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
      renderWindow->GetRenderingManager()->RequestUpdateAll();
    }

    m_CurrentImageWidth = curOutput->GetDimension(0);
    m_CurrentImageHeight = curOutput->GetDimension(1);
  }

  m_FrameCounter ++;
  if (m_FrameCounter == 10)
  {
    int nMilliseconds = m_Clock.restart();
    int fps = 10000.0f / (nMilliseconds );
    m_Controls.m_FramerateLabel->setText("Current Framerate: "+ QString::number(fps) +" FPS");
    m_FrameCounter = 0;
  }
}

void UltrasoundSupport::OnClickedViewDevice()
{
  m_FrameCounter = 0;
  // We use the activity state of the timer to determine whether we are currently viewing images
  if ( ! m_Timer->isActive() ) // Activate Imaging
  {
    this->StartViewing();
  }
  else //deactivate imaging
  {
    this->StopViewing();
  }
}

void UltrasoundSupport::OnChangedFramerateLimit(int value)
{
  m_Timer->setInterval(1000 / value);
}

void UltrasoundSupport::OnClickedFreezeButton()
{
  if ( m_Device->GetIsFreezed() )
  {
    m_Device->SetIsFreezed(false);
    m_Controls.m_FreezeButton->setText("Freeze");
  }
  else
  {
    m_Device->SetIsFreezed(true);
    m_Controls.m_FreezeButton->setText("Start Viewing Again");
  }
}

void UltrasoundSupport::OnNewDeviceWidgetDone()
{
  m_Controls.m_NewVideoDeviceWidget->setVisible(false);
  m_Controls.m_DeviceManagerWidget->setVisible(true);
  m_Controls.m_Headline->setText("Ultrasound Devices:");
  m_Controls.m_WidgetActiveDevices->setVisible(true);
}

void UltrasoundSupport::StartViewing()
{
  m_ImageAlreadySetToNode = false;

  m_Controls.tabWidget->setTabEnabled(1, true);
  m_Controls.tabWidget->setCurrentIndex(1);

  //get device & set data node
  m_Device = m_Controls.m_ActiveVideoDevices->GetSelectedService<mitk::USDevice>();
  if (m_Device.IsNull())
  {
    m_Timer->stop();
    return;
  }

  //start timer
  int interval = (1000 / m_Controls.m_FrameRate->value());
  m_Timer->setInterval(interval);
  m_Timer->start();

  //change UI elements
  m_Controls.m_BtnView->setText("Stop Viewing");

  this->CreateControlWidgets();
}

void UltrasoundSupport::StopViewing()
{
  m_Controls.tabWidget->setTabEnabled(1, false);

  this->RemoveControlWidgets();

  //stop timer & release data
  m_Timer->stop();
  m_Node->ReleaseData();
  this->RequestRenderWindowUpdate();

  //change UI elements
  m_Controls.m_BtnView->setText("Start Viewing");
}

void UltrasoundSupport::CreateControlWidgets()
{
  m_ControlProbesWidget = new QmitkUSControlsProbesWidget(m_Device->GetControlInterfaceProbes(), m_Controls.m_ToolBoxControlWidgets);
  m_Controls.probesWidgetContainer->addWidget(m_ControlProbesWidget);

  // create b mode widget for current device
  m_ControlBModeWidget = new QmitkUSControlsBModeWidget(m_Device->GetControlInterfaceBMode(), m_Controls.m_ToolBoxControlWidgets);
  m_Controls.m_ToolBoxControlWidgets->addItem(m_ControlBModeWidget, "B Mode Controls");
  if ( ! m_Device->GetControlInterfaceBMode() )
  {
    m_Controls.m_ToolBoxControlWidgets->setItemEnabled(m_Controls.m_ToolBoxControlWidgets->count()-1, false);
  }

  // create doppler widget for current device
  m_ControlDopplerWidget = new QmitkUSControlsDopplerWidget(m_Device->GetControlInterfaceDoppler(), m_Controls.m_ToolBoxControlWidgets);
  m_Controls.m_ToolBoxControlWidgets->addItem(m_ControlDopplerWidget, "Doppler Controls");
  if ( ! m_Device->GetControlInterfaceDoppler() )
  {
    m_Controls.m_ToolBoxControlWidgets->setItemEnabled(m_Controls.m_ToolBoxControlWidgets->count()-1, false);
  }

  ctkPluginContext* pluginContext = mitk::PluginActivator::GetContext();
  if ( pluginContext )
  {
    std::string filter = "(ork.mitk.services.UltrasoundCustomWidget.deviceClass=" + m_Device->GetDeviceClass() + ")";

    QString interfaceName ( us_service_interface_iid<QmitkUSAbstractCustomWidget>() );
    m_CustomWidgetServiceReference = pluginContext->getServiceReferences(interfaceName, QString::fromStdString(filter));

    if (m_CustomWidgetServiceReference.size() > 0)
    {
      m_ControlCustomWidget = pluginContext->getService<QmitkUSAbstractCustomWidget>
        (m_CustomWidgetServiceReference.at(0))->CloneForQt(m_Controls.tab2);
      m_ControlCustomWidget->SetDevice(m_Device);
      m_Controls.m_ToolBoxControlWidgets->addItem(m_ControlCustomWidget, "Custom Controls");
    }
    else
    {
      m_Controls.m_ToolBoxControlWidgets->addItem(new QWidget(m_Controls.m_ToolBoxControlWidgets), "Custom Controls");
      m_Controls.m_ToolBoxControlWidgets->setItemEnabled(m_Controls.m_ToolBoxControlWidgets->count()-1, false);
    }
  }

  // select first enabled control widget
  for ( int n = 0; n < m_Controls.m_ToolBoxControlWidgets->count(); ++n)
  {
    if ( m_Controls.m_ToolBoxControlWidgets->isItemEnabled(n) )
    {
      m_Controls.m_ToolBoxControlWidgets->setCurrentIndex(n);
      break;
    }
  }
}

void UltrasoundSupport::RemoveControlWidgets()
{
  // remove all control widgets from the tool box widget
  while (m_Controls.m_ToolBoxControlWidgets->count() > 0)
  {
    m_Controls.m_ToolBoxControlWidgets->removeItem(0);
  }

  // remove probes widget (which is not part of the tool box widget)
  m_Controls.probesWidgetContainer->removeWidget(m_ControlProbesWidget);
  delete m_ControlProbesWidget;
  m_ControlProbesWidget = 0;

  delete m_ControlBModeWidget;
  m_ControlBModeWidget = 0;

  delete m_ControlDopplerWidget;
  m_ControlDopplerWidget = 0;

  // delete custom widget if it is present
  if ( m_ControlCustomWidget )
  {
    ctkPluginContext* pluginContext = mitk::PluginActivator::GetContext();
    delete m_ControlCustomWidget; m_ControlCustomWidget = 0;

    if ( m_CustomWidgetServiceReference.size() > 0 )
    {
      pluginContext->ungetService(m_CustomWidgetServiceReference.at(0));
    }
  }
}

void UltrasoundSupport::OnDeciveServiceEvent(const ctkServiceEvent event)
{
  if ( m_Device.IsNull() || event.getType() != us::ServiceEvent::MODIFIED ) { return; }

  ctkServiceReference service = event.getServiceReference();

  if ( m_Device->GetDeviceManufacturer() != service.getProperty(mitk::USImageMetadata::PROP_DEV_MANUFACTURER).toString().toStdString()
    && m_Device->GetDeviceModel() != service.getProperty(mitk::USImageMetadata::PROP_DEV_MODEL).toString().toStdString() )
  {
    return;
  }

  if ( ! m_Device->GetIsActive() && m_Timer->isActive() )
  {
    this->StopViewing();
  }
}

UltrasoundSupport::UltrasoundSupport()
: m_ControlCustomWidget(0), m_ControlBModeWidget(0),
  m_ControlProbesWidget(0), m_ImageAlreadySetToNode(false),
  m_CurrentImageWidth(0), m_CurrentImageHeight(0)
{
  ctkPluginContext* pluginContext = mitk::PluginActivator::GetContext();

  if ( pluginContext )
  {
    // to be notified about service event of an USDevice
    pluginContext->connectServiceListener(this, "OnDeciveServiceEvent",
      QString::fromStdString("(" + us::ServiceConstants::OBJECTCLASS() + "=" + us_service_interface_iid<mitk::USDevice>() + ")"));
  }
}

UltrasoundSupport::~UltrasoundSupport()
{
}
