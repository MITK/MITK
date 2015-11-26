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

const std::string QmitkTrackingDeviceConfigurationWidget::VIEW_ID = "org.mitk.views.trackingdeviceconfigurationwidget";

QmitkTrackingDeviceConfigurationWidget::QmitkTrackingDeviceConfigurationWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
  , m_DeviceTypeCollection(nullptr)
  , m_DeviceWidgetCollection()
{
  //initializations
  m_Controls = NULL;
  CreateQtPartControl(this);
  CreateConnections();

  //Add widgets of standard tracking devices
  m_auroraWidget = new QmitkNDIAuroraWidget;
  m_polarisWidget = new QmitkNDIPolarisWidget;
  m_microntrackerWidget = new QmitkMicronTrackerWidget;
  m_optitrackWidget = new QmitkNPOptitrackWidget;
  m_virtualtrackerWidget = new QmitkVirtualTrackerWidget;
  m_openIGTLinkWidget = new QmitkOpenIGTLinkWidget;

  m_DeviceWidgetCollection.RegisterTrackingDeviceWidget(mitk::TRACKING_DEVICE_IDENTIFIER_AURORA, m_auroraWidget);
  m_DeviceWidgetCollection.RegisterTrackingDeviceWidget(mitk::TRACKING_DEVICE_IDENTIFIER_POLARIS, m_polarisWidget);
  m_DeviceWidgetCollection.RegisterTrackingDeviceWidget(mitk::TRACKING_DEVICE_IDENTIFIER_MICRON, m_microntrackerWidget);
  m_DeviceWidgetCollection.RegisterTrackingDeviceWidget(mitk::TRACKING_DEVICE_IDENTIFIER_OPTITRACK, m_optitrackWidget);
  m_DeviceWidgetCollection.RegisterTrackingDeviceWidget(mitk::TRACKING_DEVICE_IDENTIFIER_VIRTUAL, m_virtualtrackerWidget);
  m_DeviceWidgetCollection.RegisterTrackingDeviceWidget(mitk::TRACKING_DEVICE_IDENTIFIER_OPENIGTLINK, m_openIGTLinkWidget);
  m_DeviceWidgetCollection.RegisterAsMicroservice();

  RefreshTrackingDeviceCollection();

  //initialize a few UI elements
  this->m_TrackingDeviceConfigurated = false;
  AddOutput("<br>First Element selected"); //Order from Collection List

  m_Controls->m_TrackingDeviceChooser->setCurrentIndex(0);
  const std::string selectedDeviceType = m_Controls->m_TrackingDeviceChooser->currentText().toStdString();
  m_Controls->m_TrackingSystemWidget->setCurrentWidget(m_DeviceWidgetCollection.GetTrackingDeviceWidget(selectedDeviceType));

  //reset a few things
  ResetOutput();

  //restore old UI settings
  //zzz  LoadUISettings();
}

QmitkTrackingDeviceConfigurationWidget::~QmitkTrackingDeviceConfigurationWidget()
{
  StoreUISettings();
  m_DeviceWidgetCollection.UnRegisterMicroservice();
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
    connect((QObject*)(m_Controls->m_RefreshTrackingDeviceCollection), SIGNAL(clicked()), this, SLOT(RefreshTrackingDeviceCollection()));
  }
}

void QmitkTrackingDeviceConfigurationWidget::TrackingDeviceChanged()
{
  //show the correspondig widget

  const std::string selectedDeviceType = m_Controls->m_TrackingDeviceChooser->currentText().toStdString();

  m_Controls->m_TrackingSystemWidget->setCurrentWidget(m_DeviceWidgetCollection.GetTrackingDeviceWidget(selectedDeviceType));

  //the new trackingdevice is not configurated yet
  m_TrackingDeviceConfigurated = false;

  //reset output
  ResetOutput();

  AddOutput("<br>");
  AddOutput(selectedDeviceType);
  AddOutput(" selected");
  if (!m_DeviceWidgetCollection.GetTrackingDeviceWidget(selectedDeviceType)->IsDeviceInstalled())
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
      QWidget* current = m_DeviceWidgetCollection.GetTrackingDeviceWidget(name);
      if (current == nullptr)
      {
        MITK_WARN << "No widget for tracking device type " << name << " available. Please implement and register it!";
        current = new QWidget;
      }
      m_Controls->m_TrackingSystemWidget->addWidget(current);
    }
  }
}

void QmitkTrackingDeviceConfigurationWidget::Finished()
{
  m_TrackingDevice = ConstructTrackingDevice();
  m_Controls->m_TrackingSystemWidget->setEnabled(false);
  m_Controls->m_TrackingDeviceChooser->setEnabled(false);
  m_Controls->choose_tracking_device_label->setEnabled(false);
  this->m_TrackingDeviceConfigurated = true;
  emit TrackingDeviceConfigurationFinished();
}

void QmitkTrackingDeviceConfigurationWidget::Reset()
{
  m_TrackingDevice = NULL;
  m_Controls->m_TrackingSystemWidget->setEnabled(true);
  m_Controls->m_TrackingDeviceChooser->setEnabled(true);
  m_Controls->choose_tracking_device_label->setEnabled(true);
  this->m_TrackingDeviceConfigurated = false;
  emit TrackingDeviceConfigurationReseted();
}

void QmitkTrackingDeviceConfigurationWidget::ResetByUser()
{
  Reset();
}

//######################### internal help methods #######################################
void QmitkTrackingDeviceConfigurationWidget::ResetOutput()
{
  const std::string selectedDeviceType = m_Controls->m_TrackingDeviceChooser->currentText().toStdString();
  QmitkAbstractTrackingDeviceWidget* currentWidget = m_DeviceWidgetCollection.GetTrackingDeviceWidget(selectedDeviceType);
  currentWidget->ResetOutput();
}
void QmitkTrackingDeviceConfigurationWidget::AddOutput(std::string s)
{
  const std::string selectedDeviceType = m_Controls->m_TrackingDeviceChooser->currentText().toStdString();
  QmitkAbstractTrackingDeviceWidget* currentWidget = m_DeviceWidgetCollection.GetTrackingDeviceWidget(selectedDeviceType);
  currentWidget->AddOutput(s);
  currentWidget->repaint();
}
mitk::TrackingDevice::Pointer QmitkTrackingDeviceConfigurationWidget::ConstructTrackingDevice()
{
  mitk::TrackingDevice::Pointer returnValue;
  const std::string selectedDeviceType = m_Controls->m_TrackingDeviceChooser->currentText().toStdString();
  QmitkAbstractTrackingDeviceWidget* currentWidget = m_DeviceWidgetCollection.GetTrackingDeviceWidget(selectedDeviceType);
  returnValue = currentWidget->ConstructTrackingDevice();

  return returnValue;
}

mitk::TrackingDevice::Pointer QmitkTrackingDeviceConfigurationWidget::GetTrackingDevice()
{
  m_TrackingDevice = ConstructTrackingDevice();
  if (m_TrackingDevice.IsNull() || !m_TrackingDevice->IsDeviceInstalled()) return NULL;
  else return this->m_TrackingDevice;
}

bool QmitkTrackingDeviceConfigurationWidget::GetTrackingDeviceConfigured()
{
  return this->m_TrackingDeviceConfigurated;
}

void QmitkTrackingDeviceConfigurationWidget::ConfigurationFinished()
{
  Finished();
}

void QmitkTrackingDeviceConfigurationWidget::StoreUISettings()
{
  std::string id = "org.mitk.modules.igt.ui.trackingdeviceconfigurationwidget";
  const QString selectedDeviceType = m_Controls->m_TrackingDeviceChooser->currentText();

  if (this->GetPeristenceService()) // now save the settings using the persistence service
  {
    mitk::PropertyList::Pointer propList = this->GetPeristenceService()->GetPropertyList(id);
    //zzz   propList->Set("PolarisPortWin",m_Controls->m_portSpinBoxPolaris->value());
    //yyy  propList->Set("AuroraPortWin",m_Controls->m_portSpinBoxAurora->value());
    //zzz  propList->Set("PortTypePolaris", m_Controls->portTypePolaris->currentIndex());
    //yyy  propList->Set("PortTypeAurora", m_Controls->portTypeAurora->currentIndex());
    //zzzpropList->Set("MTCalibrationFile",m_MTCalibrationFile);
    propList->Set("SelectedDevice", selectedDeviceType.toStdString());
  }
  else // QSettings as a fallback if the persistence service is not available
  {
    QSettings settings;
    settings.beginGroup(QString::fromStdString(id));
    settings.setValue("trackingDeviceChooser", QVariant(selectedDeviceType));
    //yyy  settings.setValue("portSpinBoxAurora", QVariant(m_Controls->m_portSpinBoxAurora->value()));
    //zzz   settings.setValue("portSpinBoxPolaris", QVariant(m_Controls->m_portSpinBoxPolaris->value()));
    //zzz   settings.setValue("portTypePolaris", QVariant(m_Controls->portTypePolaris->currentIndex()));
    //yyy  settings.setValue("portTypeAurora", QVariant(m_Controls->portTypeAurora->currentIndex()));
    //zzz settings.setValue("mTCalibrationFile", QVariant(QString::fromStdString(m_MTCalibrationFile)));
    settings.endGroup();
  }
}

void QmitkTrackingDeviceConfigurationWidget::LoadUISettings()
{
  std::string id = "org.mitk.modules.igt.ui.trackingdeviceconfigurationwidget";
  std::string selectedDevice;
  if (this->GetPeristenceService())
  {
    mitk::PropertyList::Pointer propList = this->GetPeristenceService()->GetPropertyList(id);
    if (propList.IsNull())
    {
      MITK_ERROR << "Property list for this UI (" << id << ") is not available, could not load UI settings!"; return;
    }
    int portPolarisWin, portAuroraWin, portTypePolaris, portTypeAurora;
    propList->Get("PolarisPortWin", portPolarisWin);
    propList->Get("AuroraPortWin", portAuroraWin);
    propList->Get("PortTypePolaris", portTypePolaris);
    propList->Get("PortTypeAurora", portTypeAurora);
    //zzz propList->Get("MTCalibrationFile",m_MTCalibrationFile);
    propList->Get("SelectedDevice", selectedDevice);

    if (selectedDevice.empty())
    {
      MITK_ERROR << "Loaded data from persistence service is invalid (SelectedDevice:" << selectedDevice << "): aborted to restore data!";
      return;
    }
    //zzz   m_Controls->m_portSpinBoxPolaris->setValue(portPolarisWin);
    //yyy  m_Controls->m_portSpinBoxAurora->setValue(portAuroraWin);
    //zzz  m_Controls->portTypePolaris->setCurrentIndex(portTypePolaris);
    //yyy  m_Controls->portTypeAurora->setCurrentIndex(portTypeAurora);
    MITK_INFO << "Successfully restored UI settings";
  }
  else
  {
    // QSettings as a fallback if the persistence service is not available
    QSettings settings;
    settings.beginGroup(QString::fromStdString(id));

    selectedDevice = settings.value("trackingDeviceChooser", "").toString().toStdString();
    //yyy   m_Controls->m_portSpinBoxAurora->setValue(settings.value("portSpinBoxAurora", 0).toInt());
    //zzz   m_Controls->m_portSpinBoxPolaris->setValue(settings.value("portSpinBoxPolaris", 0).toInt());
    //zzz  m_Controls->portTypePolaris->setCurrentIndex(settings.value("portTypePolaris", 0).toInt());
    //yyy   m_Controls->portTypeAurora->setCurrentIndex(settings.value("portTypeAurora", 0).toInt());
    //zzz m_MTCalibrationFile = settings.value("mTCalibrationFile", "").toString().toStdString();

    settings.endGroup();
  }

  //the selected device requires some checks because a device that is not installed should not be restored to avoids bugs
  std::string selectedDeviceChecked = selectedDevice;
  if (!m_DeviceWidgetCollection.GetTrackingDeviceWidget(selectedDeviceChecked)->IsDeviceInstalled())
  {
    selectedDeviceChecked = mitk::TRACKING_DEVICE_IDENTIFIER_POLARIS;
  } //Default: Polaris...

  const int index = m_Controls->m_TrackingDeviceChooser->findText(QString::fromStdString(selectedDeviceChecked));

  if (index >= 0)
  {
    m_Controls->m_TrackingDeviceChooser->setCurrentIndex(index);
  }
  else
  {
    MITK_ERROR << "Failed to load UI setting for tracking device configuration";
    return;
  }

  // TODO: load corresponding widget

  m_Controls->m_TrackingSystemWidget->setCurrentWidget(m_DeviceWidgetCollection.GetTrackingDeviceWidget(selectedDeviceChecked));

  //zzz m_Controls->m_MTCalibrationFile->setText("Calibration File: " + QString::fromStdString(m_MTCalibrationFile));
}