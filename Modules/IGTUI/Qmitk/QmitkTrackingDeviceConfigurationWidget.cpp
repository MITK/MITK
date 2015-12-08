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

#include "QmitkTrackingDeviceConfigurationWidget.h"

#include <mitkSerialCommunication.h>
#include <qscrollbar.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <mitkIGTException.h>
#include <QSettings>

#include "QmitkAbstractTrackingDeviceWidget.h"

//All Tracking devices, which should be available by default
#include "mitkNDIAuroraTypeInformation.h"
#include "mitkNDIPolarisTypeInformation.h"
#include "mitkVirtualTrackerTypeInformation.h"
#include "mitkMicronTrackerTypeInformation.h"
#include "mitkNPOptitrackTrackingTypeInformation.h"
#include "mitkOpenIGTLinkTypeInformation.h"
//standard tracking devices, which always should be avaiable
#include "QmitkNDIAuroraWidget.h"
#include "QmitkNDIPolarisWidget.h"
#include "QmitkMicronTrackerWidget.h"
#include "QmitkNPOptitrackWidget.h"
#include "QmitkVirtualTrackerWidget.h"
#include "QmitkOpenIGTLinkWidget.h"

const std::string QmitkTrackingDeviceConfigurationWidget::VIEW_ID = "org.mitk.views.trackingdeviceconfigurationwidget";

QmitkTrackingDeviceConfigurationWidget::QmitkTrackingDeviceConfigurationWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
  , m_DeviceTypeCollection(nullptr)
  , m_DeviceWidgetCollection(nullptr)
{
  //initializations
  m_Controls = NULL;
  CreateQtPartControl(this);
  CreateConnections();

  us::ModuleContext* context = us::GetModuleContext();
  std::vector<us::ServiceReference<mitk::TrackingDeviceWidgetCollection> > refs = context->GetServiceReferences<mitk::TrackingDeviceWidgetCollection>();
  if (refs.empty())
  {
    MITK_ERROR << "No tracking device service found!";
  }
  m_DeviceWidgetCollection = context->GetService<mitk::TrackingDeviceWidgetCollection>(refs.front());

  //Add widgets of standard tracking devices
  m_DeviceWidgetCollection->RegisterTrackingDeviceWidget(mitk::NDIAuroraTypeInformation::GetTrackingDeviceName(), new QmitkNDIAuroraWidget);
  m_DeviceWidgetCollection->RegisterTrackingDeviceWidget(mitk::NDIPolarisTypeInformation::GetTrackingDeviceName(), new QmitkNDIPolarisWidget);
  m_DeviceWidgetCollection->RegisterTrackingDeviceWidget(mitk::MicronTrackerTypeInformation::GetTrackingDeviceName(), new QmitkMicronTrackerWidget);
  m_DeviceWidgetCollection->RegisterTrackingDeviceWidget(mitk::NPOptitrackTrackingTypeInformation::GetTrackingDeviceName(), new QmitkNPOptitrackWidget);
  m_DeviceWidgetCollection->RegisterTrackingDeviceWidget(mitk::VirtualTrackerTypeInformation::GetTrackingDeviceName(), new QmitkVirtualTrackerWidget);
  m_DeviceWidgetCollection->RegisterTrackingDeviceWidget(mitk::OpenIGTLinkTypeInformation::GetTrackingDeviceName(), new QmitkOpenIGTLinkWidget);

  RefreshTrackingDeviceCollection();

  //initialize a few UI elements
  AddOutput("<br>First Element selected"); //Order from Collection List

  m_Controls->m_TrackingDeviceChooser->setCurrentIndex(0);
  m_Controls->m_TrackingSystemWidget->setCurrentWidget(GetCurrentWidget());

  //reset a few things
  ResetOutput();

  //restore old UI settings
  LoadUISettings();
}

QmitkTrackingDeviceConfigurationWidget::~QmitkTrackingDeviceConfigurationWidget()
{
  StoreUISettings();
}

void QmitkTrackingDeviceConfigurationWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkTrackingDeviceConfigurationWidgetControls;
    m_Controls->setupUi(parent);
  }
}

void QmitkTrackingDeviceConfigurationWidget::CreateConnections()
{
  if (m_Controls)
  {
    connect((QObject*)(m_Controls->m_TrackingDeviceChooser), SIGNAL(currentIndexChanged(int)), this, SLOT(TrackingDeviceChanged()));
  }
}

void QmitkTrackingDeviceConfigurationWidget::TrackingDeviceChanged()
{
  //show the correspondig widget
  m_Controls->m_TrackingSystemWidget->setCurrentWidget(GetCurrentWidget());

  //reset output
  ResetOutput();

  AddOutput("<br>");
  AddOutput(m_Controls->m_TrackingDeviceChooser->currentText().toStdString());
  AddOutput(" selected");
  if (GetCurrentWidget() == nullptr || !GetCurrentWidget()->IsDeviceInstalled())
  {
    AddOutput("<br>ERROR: not installed!");
  }

  emit TrackingDeviceSelectionChanged();
}

void QmitkTrackingDeviceConfigurationWidget::RefreshTrackingDeviceCollection()
{
  us::ModuleContext* context = us::GetModuleContext();
  std::vector<us::ServiceReference<mitk::TrackingDeviceTypeCollection> > refs = context->GetServiceReferences<mitk::TrackingDeviceTypeCollection>();
  if (refs.empty())
  {
    MITK_ERROR << "No tracking device service found!";
  }
  m_DeviceTypeCollection = context->GetService<mitk::TrackingDeviceTypeCollection>(refs.front());

  for (auto name : m_DeviceTypeCollection->GetTrackingDeviceTypeNames())
  {
    //if the device is not included yet, add name to comboBox and widget to stackedWidget
    if (m_Controls->m_TrackingDeviceChooser->findText(QString::fromStdString(name)) == -1)
    {
      m_Controls->m_TrackingDeviceChooser->addItem(QString::fromStdString(name));
      QWidget* current = m_DeviceWidgetCollection->GetTrackingDeviceWidget(name);
      if (current == nullptr)
      {
        MITK_WARN << "No widget for tracking device type " << name << " available. Please implement and register it!";
        current = new QWidget;
      }
      m_Controls->m_TrackingSystemWidget->addWidget(current);
    }
  }
}

//######################### internal help methods #######################################
void QmitkTrackingDeviceConfigurationWidget::ResetOutput()
{
  if (GetCurrentWidget() == nullptr)
    return;
  GetCurrentWidget()->ResetOutput();
  GetCurrentWidget()->repaint();
}
void QmitkTrackingDeviceConfigurationWidget::AddOutput(std::string s)
{
  if (GetCurrentWidget() == nullptr)
    return;
  GetCurrentWidget()->AddOutput(s);
  GetCurrentWidget()->repaint();
}
mitk::TrackingDevice::Pointer QmitkTrackingDeviceConfigurationWidget::ConstructTrackingDevice()
{
  if (GetCurrentWidget() == nullptr)
    return nullptr;
  return GetCurrentWidget()->ConstructTrackingDevice();
}

mitk::TrackingDevice::Pointer QmitkTrackingDeviceConfigurationWidget::GetTrackingDevice()
{
  m_TrackingDevice = ConstructTrackingDevice();
  if (m_TrackingDevice.IsNull() || !m_TrackingDevice->IsDeviceInstalled()) return NULL;
  else return this->m_TrackingDevice;
}

void QmitkTrackingDeviceConfigurationWidget::StoreUISettings()
{
  std::string id = "org.mitk.modules.igt.ui.trackingdeviceconfigurationwidget";

  std::string selectedDevice = m_Controls->m_TrackingDeviceChooser->currentText().toStdString();

  //Save settings for every widget
  //Don't use m_DeviceTypeCollection here, it's already unregistered, when deconstructor is called...
  for (int i = 0; i < m_Controls->m_TrackingDeviceChooser->count(); i++)
  {
    if (m_DeviceWidgetCollection->GetTrackingDeviceWidget(m_Controls->m_TrackingDeviceChooser->itemText(i).toStdString()) != nullptr)
      m_DeviceWidgetCollection->GetTrackingDeviceWidget(m_Controls->m_TrackingDeviceChooser->itemText(i).toStdString())->StoreUISettings();
  }

  if (this->GetPeristenceService()) // now save the settings using the persistence service
  {
    mitk::PropertyList::Pointer propList = this->GetPeristenceService()->GetPropertyList(id);
    propList->Set("SelectedDevice", selectedDevice);
  }
  else // QSettings as a fallback if the persistence service is not available
  {
    QSettings settings;
    settings.beginGroup(QString::fromStdString(id));
    settings.setValue("trackingDeviceChooser", QVariant(QString::fromStdString(selectedDevice)));
    settings.endGroup();
  }
}

void QmitkTrackingDeviceConfigurationWidget::LoadUISettings()
{
  //Load settings for every widget
  for (auto name : m_DeviceTypeCollection->GetTrackingDeviceTypeNames())
  {
    if (m_DeviceWidgetCollection->GetTrackingDeviceWidget(name) != nullptr)
      m_DeviceWidgetCollection->GetTrackingDeviceWidget(name)->LoadUISettings();
  }

  std::string id = "org.mitk.modules.igt.ui.trackingdeviceconfigurationwidget";
  std::string selectedDevice;
  if (this->GetPeristenceService())
  {
    mitk::PropertyList::Pointer propList = this->GetPeristenceService()->GetPropertyList(id);
    if (propList.IsNull())
    {
      MITK_ERROR << "Property list for this UI (" << id << ") is not available, could not load UI settings!"; return;
    }

    propList->Get("SelectedDevice", m_Controls->m_TrackingDeviceChooser->currentText().toStdString());

    if (selectedDevice.empty())
    {
      MITK_ERROR << "Loaded data from persistence service is invalid (SelectedDevice:" << selectedDevice << "): aborted to restore data!";
      return;
    }

    MITK_INFO << "Successfully restored UI settings";
  }
  else
  {
    // QSettings as a fallback if the persistence service is not available
    QSettings settings;
    settings.beginGroup(QString::fromStdString(id));

    selectedDevice = settings.value("trackingDeviceChooser", "").toString().toStdString();

    settings.endGroup();
  }

  //the selected device requires some checks because a device that is not installed should not be restored to avoids bugs
  if (m_DeviceWidgetCollection->GetTrackingDeviceWidget(selectedDevice) == nullptr || !m_DeviceWidgetCollection->GetTrackingDeviceWidget(selectedDevice)->IsDeviceInstalled())
  {
    selectedDevice = mitk::NDIPolarisTypeInformation::GetTrackingDeviceName();
  } //Default: Polaris...

  const int index = m_Controls->m_TrackingDeviceChooser->findText(QString::fromStdString(selectedDevice));

  if (index >= 0)
  {
    m_Controls->m_TrackingDeviceChooser->setCurrentIndex(index);
  }
  else
  {
    MITK_ERROR << "Failed to load UI setting for tracking device configuration";
    return;
  }

  m_Controls->m_TrackingSystemWidget->setCurrentWidget(GetCurrentWidget());
}

QmitkAbstractTrackingDeviceWidget* QmitkTrackingDeviceConfigurationWidget::GetCurrentWidget()
{
  return m_DeviceWidgetCollection->GetTrackingDeviceWidget(m_Controls->m_TrackingDeviceChooser->currentText().toStdString());
}