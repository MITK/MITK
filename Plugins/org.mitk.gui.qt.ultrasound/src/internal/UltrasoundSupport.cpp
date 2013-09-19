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

#include <ctkFlowLayout.h>
#include <ctkDoubleSlider.h>

const std::string UltrasoundSupport::VIEW_ID = "org.mitk.views.ultrasoundsupport";

void UltrasoundSupport::SetFocus()
{
  //m_Controls.m_AddDevice->setFocus();
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
  connect( m_Timer, SIGNAL(timeout()), this, SLOT(DisplayImage()));

  // Initializations
  m_Controls.m_NewVideoDeviceWidget->setVisible(false);
  std::string filter = "(&(" + us::ServiceConstants::OBJECTCLASS() + "=" + "org.mitk.services.UltrasoundDevice)(" + mitk::USDevice::US_PROPKEY_ISACTIVE + "=true))";
  m_Controls.m_ActiveVideoDevices->Initialize<mitk::USDevice>(mitk::USDevice::US_PROPKEY_LABEL ,filter);

  m_Node = mitk::DataNode::New();
  m_Node->SetName("US Image Stream");
  this->GetDataStorage()->Add(m_Node);

  ctkFlowLayout* flowLayout = ctkFlowLayout::replaceLayout(m_Controls.m_WidgetDevices);
  flowLayout->setAlignItems(true);
  flowLayout->setOrientation(Qt::Vertical);
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
  m_Device->UpdateOutputData(0);
  m_Node->SetData(m_Device->GetOutput());
  this->RequestRenderWindowUpdate();

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
    //get device & set data node
    m_Device = m_Controls.m_ActiveVideoDevices->GetSelectedService<mitk::USDevice>();
    if (m_Device.IsNull()){
      m_Timer->stop();
      return;
    }
    m_Device->Update();
    m_Node->SetData(m_Device->GetOutput());

    //start timer
    int interval = (1000 / m_Controls.m_FrameRate->value());
    m_Timer->setInterval(interval);
    m_Timer->start();

    //reinit view
    GlobalReinit();

    //change UI elements
    m_Controls.m_BtnView->setText("Stop Viewing");
    m_Controls.m_FrameRate->setEnabled(false);

    m_ControlProbesWidget = new QmitkUSControlsProbesWidget(m_Device->GetControlInterfaceProbes(), m_Controls.tab2);
    //m_Controls.tab2->layout()->addWidget(m_ControlProbesWidget);
    m_Controls.m_ToolBoxControlWidgets->addItem(m_ControlProbesWidget, "Probes Controls");

    m_ControlBModeWidget = new QmitkUSControlsBModeWidget(m_Device->GetControlInterfaceBMode(), m_Controls.tab2);
    //m_Controls.tab2->layout()->addWidget(m_ControlBModeWidget);
    m_Controls.m_ToolBoxControlWidgets->addItem(m_ControlBModeWidget, "B Mode Controls");

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
        //m_Controls.tab2->layout()->addWidget(m_ControlCustomWidget);
        m_Controls.m_ToolBoxControlWidgets->addItem(m_ControlCustomWidget, "Custom Controls");
      }
    }
  }
  else //deactivate imaging
  {
    while (m_Controls.m_ToolBoxControlWidgets->count() > 0)
    {
      m_Controls.m_ToolBoxControlWidgets->removeItem(0);
    }

    //m_Controls.tab2->layout()->removeWidget(m_ControlProbesWidget);
    delete m_ControlProbesWidget;
    m_ControlProbesWidget = 0;

    //m_Controls.tab2->layout()->removeWidget(m_ControlBModeWidget);
    delete m_ControlBModeWidget;
    m_ControlBModeWidget = 0;

    if ( m_ControlCustomWidget )
    {
      ctkPluginContext* pluginContext = mitk::PluginActivator::GetContext();
      //m_Controls.tab2->layout()->removeWidget(m_ControlCustomWidget);
      delete m_ControlCustomWidget; m_ControlCustomWidget = 0;

      if ( m_CustomWidgetServiceReference.size() > 0 )
      {
        pluginContext->ungetService(m_CustomWidgetServiceReference.at(0));
      }
    }

    //stop timer & release data
    m_Timer->stop();
    m_Node->ReleaseData();
    this->RequestRenderWindowUpdate();

    //change UI elements
    m_Controls.m_BtnView->setText("Start Viewing");
    m_Controls.m_FrameRate->setEnabled(true);
  }
}

void UltrasoundSupport::OnNewDeviceWidgetDone()
{
  m_Controls.m_NewVideoDeviceWidget->setVisible(false);
  m_Controls.m_DeviceManagerWidget->setVisible(true);
  m_Controls.m_Headline->setText("Ultrasound Devices:");
  m_Controls.m_WidgetActiveDevices->setVisible(true);
}

void UltrasoundSupport::GlobalReinit()
{
  // get all nodes that have not set "includeInBoundingBox" to false
  mitk::NodePredicateNot::Pointer pred = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("includeInBoundingBox", mitk::BoolProperty::New(false)));
  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetDataStorage()->GetSubset(pred);

  // calculate bounding geometry of these nodes
  mitk::TimeSlicedGeometry::Pointer bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(rs, "visible");

  // initialize the views to the bounding geometry
  mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
}

/*void UltrasoundSupport::OnPreferencesChanged(const berry::IBerryPreferences* prefs)
{
  std::cout << prefs << std::endl;
}*/

UltrasoundSupport::UltrasoundSupport()
  : m_ControlCustomWidget(0),
    m_ControlBModeWidget(0),
    m_ControlProbesWidget(0)
{
}

UltrasoundSupport::~UltrasoundSupport()
{
}
