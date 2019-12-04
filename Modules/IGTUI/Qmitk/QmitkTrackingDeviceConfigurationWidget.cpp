/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTrackingDeviceConfigurationWidget.h"

#include "mitkNDIPolarisTypeInformation.h"
#include "mitkNDIAuroraTypeInformation.h"

#include <QSettings>

const std::string QmitkTrackingDeviceConfigurationWidget::VIEW_ID = "org.mitk.views.trackingdeviceconfigurationwidget";

QmitkTrackingDeviceConfigurationWidget::QmitkTrackingDeviceConfigurationWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
  , m_Controls(nullptr)
  , m_DeviceToWidgetIndexMap()
{
  //initializations
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
  delete m_Controls;
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
  const std::string currentDevice = this->GetCurrentDeviceName();

  //show the correspondig widget
  m_Controls->m_TrackingSystemWidget->setCurrentIndex(m_DeviceToWidgetIndexMap[currentDevice]);

  //reset output
  ResetOutput();

  AddOutput("<br>");
  AddOutput(currentDevice);
  AddOutput(" selected");

  QmitkAbstractTrackingDeviceWidget* widget = GetWidget(currentDevice);

  if (widget == nullptr || !widget->IsDeviceInstalled())
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

  // get tracking device type service references
  us::ModuleContext* context = us::GetModuleContext();
  std::vector<us::ServiceReference<mitk::TrackingDeviceTypeCollection> > deviceRefs =
    context->GetServiceReferences<mitk::TrackingDeviceTypeCollection>();

  if (deviceRefs.empty())
  {
    MITK_ERROR << "No tracking device type service found!";
    return;
  }

  // get tracking device configuration widget service references
  std::vector<us::ServiceReference<mitk::TrackingDeviceWidgetCollection> > widgetRefs =
    context->GetServiceReferences<mitk::TrackingDeviceWidgetCollection>();

  if (widgetRefs.empty())
  {
    MITK_ERROR << "No tracking device configuration widget service found!";
    return;
  }

  const us::ServiceReference<mitk::TrackingDeviceTypeCollection>& deviceServiceReference = deviceRefs.front();
  const us::ServiceReference<mitk::TrackingDeviceWidgetCollection>& widgetServiceReference = widgetRefs.front();

  mitk::TrackingDeviceTypeCollection* deviceTypeCollection =
    context->GetService<mitk::TrackingDeviceTypeCollection>(deviceServiceReference);

  mitk::TrackingDeviceWidgetCollection* deviceWidgetCollection =
    context->GetService<mitk::TrackingDeviceWidgetCollection>(widgetServiceReference);

  for (auto name : deviceTypeCollection->GetTrackingDeviceTypeNames())
  {
    // if the device is not included yet, add name to comboBox and widget to stackedWidget
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

  if (!m_DeviceToWidgetIndexMap.empty())
  {
    m_Controls->m_TrackingDeviceChooser->setCurrentIndex(0);
    m_Controls->m_TrackingSystemWidget->setCurrentIndex(0);
  }

  context->UngetService(deviceServiceReference);
  context->UngetService(widgetServiceReference);
}

//######################### internal help methods #######################################
void QmitkTrackingDeviceConfigurationWidget::ResetOutput()
{
  QmitkAbstractTrackingDeviceWidget* currentWidget = this->GetWidget(this->GetCurrentDeviceName());

  if (currentWidget == nullptr)
  {
    return;
  }

  currentWidget->ResetOutput();
  currentWidget->repaint();
}

void QmitkTrackingDeviceConfigurationWidget::AddOutput(std::string s)
{
  QmitkAbstractTrackingDeviceWidget* currentWidget = this->GetWidget(this->GetCurrentDeviceName());

  if (currentWidget == nullptr)
  {
    return;
  }

  currentWidget->AddOutput(s);
  currentWidget->repaint();
}

mitk::TrackingDevice::Pointer QmitkTrackingDeviceConfigurationWidget::GetTrackingDevice()
{
  QmitkAbstractTrackingDeviceWidget* currentWidget = this->GetWidget(this->GetCurrentDeviceName());

  if (currentWidget == nullptr || !currentWidget->IsDeviceInstalled())
  {
    return nullptr;
  }

  return currentWidget->GetTrackingDevice();
}

void QmitkTrackingDeviceConfigurationWidget::StoreUISettings()
{
  std::string id = "org.mitk.modules.igt.ui.trackingdeviceconfigurationwidget";

  std::string selectedDevice = this->GetCurrentDeviceName();

  //Save settings for every widget
  //Don't use m_DeviceTypeCollection here, it's already unregistered, when deconstructor is called...
  for (int index = 0; index < m_Controls->m_TrackingSystemWidget->count(); index++)
  {
    QmitkAbstractTrackingDeviceWidget* widget = dynamic_cast<QmitkAbstractTrackingDeviceWidget*>(m_Controls->m_TrackingSystemWidget->widget(index));

    if (widget != nullptr)
    {
      widget->StoreUISettings();
    }
  }

  if (this->GetPersistenceService()) // now save the settings using the persistence service
  {
    mitk::PropertyList::Pointer propList = this->GetPersistenceService()->GetPropertyList(id);
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
 * e.g. by calling RefreshTrackingDeviceCollection() before.
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
  if (this->GetPersistenceService())
  {
    mitk::PropertyList::Pointer propList = this->GetPersistenceService()->GetPropertyList(id);
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

std::string QmitkTrackingDeviceConfigurationWidget::GetCurrentDeviceName(void) const
{
  return m_Controls->m_TrackingDeviceChooser->currentText().toStdString();
}

QmitkAbstractTrackingDeviceWidget* QmitkTrackingDeviceConfigurationWidget::GetWidget(const std::string& deviceName) const
{
  const auto& deviceIterator = m_DeviceToWidgetIndexMap.find(deviceName);

  if (deviceIterator != m_DeviceToWidgetIndexMap.end())
  {
    QWidget* widget = m_Controls->m_TrackingSystemWidget->widget(deviceIterator->second);

    return dynamic_cast<QmitkAbstractTrackingDeviceWidget*>(widget);
  }

  return nullptr;
}

void QmitkTrackingDeviceConfigurationWidget::OnConnected(bool _success)
{
  this->GetWidget(this->GetCurrentDeviceName())->OnConnected(_success);
}
void QmitkTrackingDeviceConfigurationWidget::OnDisconnected(bool _success)
{
  this->GetWidget(this->GetCurrentDeviceName())->OnDisconnected(_success);
}

void QmitkTrackingDeviceConfigurationWidget::OnStartTracking(bool _success)
{
  this->GetWidget(this->GetCurrentDeviceName())->OnStartTracking(_success);
}
void QmitkTrackingDeviceConfigurationWidget::OnStopTracking(bool _success)
{
  this->GetWidget(this->GetCurrentDeviceName())->OnStopTracking(_success);
}

void QmitkTrackingDeviceConfigurationWidget::OnToolStorageChanged()
{
  this->GetWidget(this->GetCurrentDeviceName())->OnToolStorageChanged();
}
