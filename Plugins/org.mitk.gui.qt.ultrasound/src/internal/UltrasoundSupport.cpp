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
#include <mitkImageGenerator.h>

// Qmitk
#include "UltrasoundSupport.h"

// Qt
#include <QMessageBox>
#include <QSettings>
#include <QTimer>

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
connect( m_Controls.m_ActiveVideoDevices, SIGNAL(ServiceSelectionChanged(us::ServiceReferenceU)), this, SLOT(OnChangedActiveDevice()) );
connect( m_Controls.m_RunImageTimer, SIGNAL(clicked()), this, SLOT(OnChangedActiveDevice()) );
connect( m_Controls.m_ShowImageStream, SIGNAL(clicked()), this, SLOT(OnChangedActiveDevice()) );
connect( m_Controls.m_NewVideoDeviceWidget, SIGNAL(Finished()), this, SLOT(OnNewDeviceWidgetDone()) ); // After NewDeviceWidget finished editing
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
m_Controls.m_ActiveVideoDevices->SetAutomaticallySelectFirstEntry(true);
m_FrameCounter = 0;

// Create Node for US Stream
if (m_Node.IsNull())
{
m_Node = mitk::DataNode::New();
m_Node->SetName("US Support Viewing Stream");
//create a dummy image (gray values 0..255) for correct initialization of level window, etc.
mitk::Image::Pointer dummyImage = mitk::ImageGenerator::GenerateRandomImage<float>(100, 100, 1, 1, 1, 1, 1, 255,0);
m_Node->SetData(dummyImage);
m_OldGeometry = dynamic_cast<mitk::SlicedGeometry3D*>(dummyImage->GetGeometry());
}

m_Controls.tabWidget->setTabEnabled(1, false);

LoadUISettings();
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
//Update device
m_Device->Modified();
m_Device->Update();

//Only update the view if the image is shown
if(m_Controls.m_ShowImageStream->isChecked())
{
  //Update data node
  mitk::Image::Pointer curOutput = m_Device->GetOutput();
  m_Node->SetData(curOutput);

  // if the geometry changed: reinitialize the ultrasound image
  if((m_OldGeometry.IsNotNull()) &&
     (curOutput->GetGeometry() != NULL) &&
     (!mitk::Equal(m_OldGeometry.GetPointer(),curOutput->GetGeometry(),0.0001,false))
    )
  {
    mitk::IRenderWindowPart* renderWindow = this->GetRenderWindowPart();
    if ( (renderWindow != NULL) && (curOutput->GetTimeGeometry()->IsValid()) && (m_Controls.m_ShowImageStream->isChecked()) )
    {
      renderWindow->GetRenderingManager()->InitializeViews(
      curOutput->GetGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
      renderWindow->GetRenderingManager()->RequestUpdateAll();
    }
    m_CurrentImageWidth = curOutput->GetDimension(0);
    m_CurrentImageHeight = curOutput->GetDimension(1);
    m_OldGeometry = dynamic_cast<mitk::SlicedGeometry3D*>(curOutput->GetGeometry());
  }
  //if not: only update the view
  else
  {
    this->RequestRenderWindowUpdate();
  }
}

//Update frame counter
m_FrameCounter ++;
if (m_FrameCounter >= 10)
  {
  int nMilliseconds = m_Clock.restart();
  int fps = 10000.0f / (nMilliseconds );
  m_Controls.m_FramerateLabel->setText("Current Framerate: "+ QString::number(fps) +" FPS");
  m_FrameCounter = 0;
  }
}

void UltrasoundSupport::OnChangedFramerateLimit(int value)
{
m_Timer->stop();
m_Timer->setInterval(1000 / value);
m_Timer->start();
}

void UltrasoundSupport::OnClickedFreezeButton()
{
  if ( m_Device.IsNull() )
  {
    MITK_WARN("UltrasoundSupport") << "Freeze button clicked though no device is selected.";
    return;
  }
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

void UltrasoundSupport::OnChangedActiveDevice()
{
//clean up and stop timer
m_Timer->stop();
this->RemoveControlWidgets();
this->GetDataStorage()->Remove(m_Node);
m_Node->ReleaseData();

//get current device, abort if it is invalid
m_Device = m_Controls.m_ActiveVideoDevices->GetSelectedService<mitk::USDevice>();
if (m_Device.IsNull())
{
m_Controls.tabWidget->setTabEnabled(1, false);
return;
}

//create the widgets for this device and enable the widget tab
this->CreateControlWidgets();
m_Controls.tabWidget->setTabEnabled(1, true);

//show node if the option is enabled
if(m_Controls.m_ShowImageStream->isChecked())
{this->GetDataStorage()->Add(m_Node);}

//start timer
if(m_Controls.m_RunImageTimer->isChecked())
  {
  int interval = (1000 / m_Controls.m_FrameRate->value());
  m_Timer->setInterval(interval);
  m_Timer->start();
  m_Controls.m_TimerWidget->setEnabled(true);
  }
else
  {
  m_Controls.m_TimerWidget->setEnabled(false);
  }
}

void UltrasoundSupport::OnNewDeviceWidgetDone()
{
m_Controls.m_NewVideoDeviceWidget->setVisible(false);
m_Controls.m_DeviceManagerWidget->setVisible(true);
m_Controls.m_Headline->setText("Ultrasound Devices:");
m_Controls.m_WidgetActiveDevices->setVisible(true);
}

void UltrasoundSupport::CreateControlWidgets()
{
m_ControlProbesWidget = new QmitkUSControlsProbesWidget(m_Device->GetControlInterfaceProbes(), m_Controls.m_ToolBoxControlWidgets);
m_Controls.probesWidgetContainer->addWidget(m_ControlProbesWidget);

// create b mode widget for current device
m_ControlBModeWidget = new QmitkUSControlsBModeWidget(m_Device->GetControlInterfaceBMode(), m_Controls.m_ToolBoxControlWidgets);
m_Controls.m_ToolBoxControlWidgets->addItem(m_ControlBModeWidget, "B Mode Controls");
if ( ! m_Device->GetControlInterfaceBMode() )
{m_Controls.m_ToolBoxControlWidgets->setItemEnabled(m_Controls.m_ToolBoxControlWidgets->count()-1, false);}

// create doppler widget for current device
m_ControlDopplerWidget = new QmitkUSControlsDopplerWidget(m_Device->GetControlInterfaceDoppler(), m_Controls.m_ToolBoxControlWidgets);
m_Controls.m_ToolBoxControlWidgets->addItem(m_ControlDopplerWidget, "Doppler Controls");
if ( ! m_Device->GetControlInterfaceDoppler() )
{m_Controls.m_ToolBoxControlWidgets->setItemEnabled(m_Controls.m_ToolBoxControlWidgets->count()-1, false);}

ctkPluginContext* pluginContext = mitk::PluginActivator::GetContext();
if ( pluginContext )
{
  std::string filter = "(ork.mitk.services.UltrasoundCustomWidget.deviceClass=" + m_Device->GetDeviceClass() + ")";

QString interfaceName = QString::fromStdString(us_service_interface_iid<QmitkUSAbstractCustomWidget>() );
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
if(!m_ControlProbesWidget) {return;} //widgets do not exist... nothing to do

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

if ( m_Device->GetManufacturer() != service.getProperty(QString::fromStdString(mitk::USDevice::GetPropertyKeys().US_PROPKEY_MANUFACTURER)).toString().toStdString()
&& m_Device->GetName() != service.getProperty(QString::fromStdString(mitk::USDevice::GetPropertyKeys().US_PROPKEY_NAME)).toString().toStdString() )
{
return;
}

if ( ! m_Device->GetIsActive() && m_Timer->isActive() )
{
m_Timer->stop();
}

if ( m_CurrentDynamicRange != service.getProperty(QString::fromStdString(mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_DYNAMIC_RANGE)).toDouble() )
{
m_CurrentDynamicRange = service.getProperty(QString::fromStdString(mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_DYNAMIC_RANGE)).toDouble();

// update level window for the current dynamic range
mitk::LevelWindow levelWindow;
m_Node->GetLevelWindow(levelWindow);
levelWindow.SetAuto(m_Image, true, true);
m_Node->SetLevelWindow(levelWindow);
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
  try
  {
    m_Timer->stop();

    // Get all active devicesand deactivate them to prevent freeze
    std::vector<mitk::USDevice*> devices = this->m_Controls.m_ActiveVideoDevices->GetAllServices<mitk::USDevice>();
    for (int i = 0; i < devices.size(); i ++)
    {
        mitk::USDevice::Pointer device = devices[i];
        if (device.IsNotNull() && device->GetIsActive())
        {
          device->Deactivate();
          device->Disconnect();
        }
    }

    StoreUISettings();
  }
  catch(std::exception &e)
  {
    MITK_ERROR << "Exception during call of destructor! Message: " << e.what();
  }

}

void UltrasoundSupport::StoreUISettings()
{
  QSettings settings;
  settings.beginGroup(QString::fromStdString(VIEW_ID));
  settings.setValue("DisplayImage", QVariant(m_Controls.m_ShowImageStream->isChecked()));
  settings.setValue("RunImageTimer", QVariant(m_Controls.m_RunImageTimer->isChecked()));
  settings.endGroup();
}

void UltrasoundSupport::LoadUISettings()
{
  QSettings settings;
  settings.beginGroup(QString::fromStdString(VIEW_ID));
  m_Controls.m_ShowImageStream->setChecked(settings.value("DisplayImage", true).toBool());
  m_Controls.m_RunImageTimer->setChecked(settings.value("RunImageTimer", true).toBool());
  settings.endGroup();
}
