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
  , m_DeviceToWidgetIndexMap()
{
  //initializations
  m_Controls = nullptr;
  CreateQtPartControl(this);
  CreateConnections();

  RefreshTrackingDeviceCollection();

  //initialize a few UI elements
  AddOutput("<br>First Element selected"); //Order from Collection List

  //reset a few things
  ResetOutput();

  //restore old UI settings
  LoadUISettings();
}

QmitkTrackingDeviceConfigurationWidget::~QmitkTrackingDeviceConfigurationWidget()
{
  StoreUISettings();
  m_Controls = nullptr;
  m_TrackingDevice = nullptr;
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
  // clean-up of stacked widget, drop-down box and map
  for (auto& item : m_DeviceToWidgetIndexMap)
  {
    m_Controls->m_TrackingSystemWidget->removeWidget(m_Controls->m_TrackingSystemWidget->widget(item.second));
    MITK_INFO << "removing widget for device '" << item.first << "'";
  }

  m_Controls->m_TrackingDeviceChooser->clear();

  m_DeviceToWidgetIndexMap.clear();


  // get tracking device type service
  us::ModuleContext* context = us::GetModuleContext();
  std::vector<us::ServiceReference<mitk::TrackingDeviceTypeCollection> > deviceRefs =
    context->GetServiceReferences<mitk::TrackingDeviceTypeCollection>();

  if (deviceRefs.empty())
  {
    MITK_ERROR << "No tracking device type service found!";
  }

  mitk::TrackingDeviceTypeCollection* deviceTypeCollection =
    context->GetService<mitk::TrackingDeviceTypeCollection>(deviceRefs.front());


  // get tracking device configuration widget service
  std::vector<us::ServiceReference<mitk::TrackingDeviceWidgetCollection> > widgetRefs =
    context->GetServiceReferences<mitk::TrackingDeviceWidgetCollection>();

  if (widgetRefs.empty())
  {
    MITK_ERROR << "No tracking device configuration widget service found!";
  }

  mitk::TrackingDeviceWidgetCollection* deviceWidgetCollection =
    context->GetService<mitk::TrackingDeviceWidgetCollection>(widgetRefs.front());


  for (auto name : deviceTypeCollection->GetTrackingDeviceTypeNames())
  {
    //if the device is not included yet, add name to comboBox and widget to stackedWidget
    if (m_Controls->m_TrackingDeviceChooser->findText(QString::fromStdString(name)) == -1)
    {
      m_Controls->m_TrackingDeviceChooser->addItem(QString::fromStdString(name));
      QWidget* current = deviceWidgetCollection->GetTrackingDeviceWidgetClone(name);
      if (current == nullptr)
      {
        MITK_WARN << "No widget for tracking device type " << name << " available. Please implement and register it!";
        current = new QWidget();
      }

      m_DeviceToWidgetIndexMap[name] = m_Controls->m_TrackingSystemWidget->addWidget(current);
    }
  }

  m_Controls->m_TrackingDeviceChooser->setCurrentIndex(0);
  m_Controls->m_TrackingSystemWidget->setCurrentWidget(0);
}

//######################### internal help methods #######################################
void QmitkTrackingDeviceConfigurationWidget::ResetOutput()
{
  QmitkAbstractTrackingDeviceWidget* currentWidget = GetCurrentWidget();

  if (currentWidget == nullptr)
  {
    return;
  }

  currentWidget->ResetOutput();
  currentWidget->repaint();
}
void QmitkTrackingDeviceConfigurationWidget::AddOutput(std::string s)
{
  QmitkAbstractTrackingDeviceWidget* currentWidget = GetCurrentWidget();

  if (currentWidget == nullptr)
  {
    return;
  }

  currentWidget->AddOutput(s);
  currentWidget->repaint();
}
mitk::TrackingDevice::Pointer QmitkTrackingDeviceConfigurationWidget::ConstructTrackingDevice()
{
  QmitkAbstractTrackingDeviceWidget* currentWidget = GetCurrentWidget();

  if (currentWidget == nullptr)
  {
    return nullptr;
  }

  return currentWidget->ConstructTrackingDevice();
}

mitk::TrackingDevice::Pointer QmitkTrackingDeviceConfigurationWidget::GetTrackingDevice()
{
  m_TrackingDevice = ConstructTrackingDevice();
  if (m_TrackingDevice.IsNull() || !m_TrackingDevice->IsDeviceInstalled()) return nullptr;
  else return this->m_TrackingDevice;
}

void QmitkTrackingDeviceConfigurationWidget::StoreUISettings()
{
  std::string id = "org.mitk.modules.igt.ui.trackingdeviceconfigurationwidget";

  std::string selectedDevice = m_Controls->m_TrackingDeviceChooser->currentText().toStdString();

  //Save settings for every widget
  //Don't use m_DeviceTypeCollection here, it's already unregistered, when deconstructor is called...
  for (int count = 0; count < m_Controls->m_TrackingSystemWidget->count(); count++)
  {
    QmitkAbstractTrackingDeviceWidget* widget = dynamic_cast<QmitkAbstractTrackingDeviceWidget*>(m_Controls->m_TrackingSystemWidget->widget(count));

    if (widget != nullptr)
    {
      widget->StoreUISettings();
    }
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

/**
 * @brief QmitkTrackingDeviceConfigurationWidget::LoadUISettings
 *
 * Precondition:
 * Make sure that QStackedWidget is already initialized,
 * e.g. by calling RefreshTrackingDeviceCollection().
 */
void QmitkTrackingDeviceConfigurationWidget::LoadUISettings()
{
  //Load settings for every widget
  for (int index = 0; index < m_Controls->m_TrackingSystemWidget->count(); index++)
  {
    QmitkAbstractTrackingDeviceWidget* widget = dynamic_cast<QmitkAbstractTrackingDeviceWidget*>(m_Controls->m_TrackingSystemWidget->widget(index));

    if (widget != nullptr)
    {
      widget->LoadUISettings();
    }
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

    propList->Get("SelectedDevice", selectedDevice);

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

  // The selected device requires some checks because a device that is not installed should not be restored to avoid bugs.
  // Use NDI Polaris as default if there's no widget registered for selected device or there's a widget for it but no device installed.

  const auto& deviceIterator = m_DeviceToWidgetIndexMap.find(selectedDevice);

  if (deviceIterator != m_DeviceToWidgetIndexMap.end())
  {
    QmitkAbstractTrackingDeviceWidget* widget =
      dynamic_cast<QmitkAbstractTrackingDeviceWidget*>(m_Controls->m_TrackingSystemWidget->widget(deviceIterator->second));

    if (widget == nullptr || (widget != nullptr && !widget->IsDeviceInstalled()))
    {
      selectedDevice = mitk::NDIPolarisTypeInformation::GetTrackingDeviceName();
    }
  }
  else
  {
    selectedDevice = mitk::NDIPolarisTypeInformation::GetTrackingDeviceName();
  }

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

  m_Controls->m_TrackingSystemWidget->setCurrentIndex(m_DeviceToWidgetIndexMap[selectedDevice]);
}

QmitkAbstractTrackingDeviceWidget* QmitkTrackingDeviceConfigurationWidget::GetCurrentWidget()
{
  const auto& deviceIterator = m_DeviceToWidgetIndexMap.find(m_Controls->m_TrackingDeviceChooser->currentText().toStdString());

  if (deviceIterator != m_DeviceToWidgetIndexMap.end())
  {
    QWidget* currentWidget = m_Controls->m_TrackingSystemWidget->widget(deviceIterator->second);

    return dynamic_cast<QmitkAbstractTrackingDeviceWidget*>(currentWidget);
  }

  return nullptr;
}
