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
#include "QmitkUSNavigationStepCombinedModality.h"
#include "ui_QmitkUSNavigationStepCombinedModality.h"

#include "mitkAbstractUltrasoundTrackerDevice.h"
#include "mitkUSCombinedModality.h"


#include "../Widgets/QmitkUSCombinedModalityEditWidget.h"

#include <QFileDialog>
#include <QTextStream>
#include <QSettings>

#include "mitkBaseRenderer.h"

QmitkUSNavigationStepCombinedModality::QmitkUSNavigationStepCombinedModality(QWidget *parent) :
  QmitkUSAbstractNavigationStep(parent),
  m_LastCalibrationFilename(""),
  m_CalibrationLoadedNecessary(true),
  m_ListenerDeviceChanged(this, &QmitkUSNavigationStepCombinedModality::OnDevicePropertyChanged),
  ui(new Ui::QmitkUSNavigationStepCombinedModality)
{
  ui->setupUi(this);

  // combined modality create widget should only be visible after button press
  ui->combinedModalityCreateWidget->setVisible(false);
  ui->combinedModalityEditWidget->setVisible(false);

  connect(ui->combinedModalityListWidget, SIGNAL(ServiceSelectionChanged(us::ServiceReferenceU)), this, SLOT(OnDeviceSelectionChanged()));
  connect(ui->combinedModalityListWidget, SIGNAL(ServiceModified(us::ServiceReferenceU)), this, SLOT(OnDeviceSelectionChanged()));
  connect(ui->combinedModalityCreateWidget, SIGNAL(SignalAborted()), this, SLOT(OnCombinedModalityCreationExit()));
  connect(ui->combinedModalityCreateWidget, SIGNAL(SignalCreated()), this, SLOT(OnCombinedModalityCreationExit()));

  connect(ui->combinedModalityEditWidget, SIGNAL(SignalAborted()), this, SLOT(OnCombinedModalityEditExit()));
  connect(ui->combinedModalityEditWidget, SIGNAL(SignalSaved()), this, SLOT(OnCombinedModalityEditExit()));

  std::string filterOnlyCombinedModalities = "(&(" + us::ServiceConstants::OBJECTCLASS() + "=" + "org.mitk.services.AbstractUltrasoundTrackerDevice)(" + mitk::AbstractUltrasoundTrackerDevice::US_PROPKEY_CLASS + "=" + mitk::AbstractUltrasoundTrackerDevice::DeviceClassIdentifier + "))";
  //std::string filter = "(&(" + us::ServiceConstants::OBJECTCLASS() + "=" + "org.mitk.services.UltrasoundDevice))";
  ui->combinedModalityListWidget->Initialize<mitk::AbstractUltrasoundTrackerDevice>(filterOnlyCombinedModalities);
  ui->combinedModalityListWidget->SetAutomaticallySelectFirstEntry(true);

  //try to load UI settings
  QSettings settings;
  settings.beginGroup(QString::fromStdString("QmitkUSNavigationStepCombinedModality"));
  m_LastCalibrationFilename = settings.value("LastCalibrationFilename", QVariant("")).toString().toStdString();
  MITK_DEBUG << "PERSISTENCE load: " << m_LastCalibrationFilename;
  settings.endGroup();
}

QmitkUSNavigationStepCombinedModality::~QmitkUSNavigationStepCombinedModality()
{
  ui->combinedModalityListWidget->blockSignals(true);
  //save UI settings
  QSettings settings;
  settings.beginGroup(QString::fromStdString("QmitkUSNavigationStepCombinedModality"));
  settings.setValue("LastCalibrationFilename", QVariant(m_LastCalibrationFilename.c_str()));
  settings.endGroup();
  MITK_DEBUG << "PERSISTENCE save: " << m_LastCalibrationFilename;
  //delete UI
  delete ui;
}

void QmitkUSNavigationStepCombinedModality::OnDeviceSelectionChanged()
{
  mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality = this->GetSelectedCombinedModality();
  bool combinedModalitySelected = combinedModality.IsNotNull();

  ui->calibrationGroupBox->setEnabled(combinedModalitySelected);
  ui->combinedModalityDeleteButton->setEnabled(combinedModalitySelected);
  ui->combinedModalitEditButton->setEnabled(combinedModalitySelected);

  if (!combinedModalitySelected || m_CombinedModality != combinedModality)
  {
    emit SignalNoLongerReadyForNextStep();

    if (m_CombinedModality.IsNotNull() && m_CombinedModality->GetUltrasoundDevice().IsNotNull())
    {
      m_CombinedModality->GetUltrasoundDevice()->RemovePropertyChangedListener(m_ListenerDeviceChanged);
    }

    if (combinedModalitySelected && combinedModality->GetUltrasoundDevice().IsNotNull())
    {
      combinedModality->GetUltrasoundDevice()->RemovePropertyChangedListener(m_ListenerDeviceChanged);
    }
  }
  m_CombinedModality = combinedModality;

  if (combinedModalitySelected)
  {
    bool calibrated = this->UpdateCalibrationState();

    if (!m_CalibrationLoadedNecessary) { emit SignalReadyForNextStep(); }
    else
    {
      if (calibrated) { emit SignalReadyForNextStep(); }
      else { emit SignalNoLongerReadyForNextStep(); }
    }

    // enable disconnect button only if combined modality is connected or active
    ui->combinedModalityDistconnectButton->setEnabled(combinedModality->GetUltrasoundDevice()->GetDeviceState() >= mitk::USDevice::State_Connected);
    ui->combinedModalityActivateButton->setEnabled(combinedModality->GetUltrasoundDevice()->GetDeviceState() < mitk::USDevice::State_Activated);

    this->UpdateTrackingToolNames();
  }
  else
  {
    ui->combinedModalityDistconnectButton->setEnabled(false);
    ui->combinedModalityActivateButton->setEnabled(false);
  }
}

void QmitkUSNavigationStepCombinedModality::OnLoadCalibration()
{
  QString filename = QFileDialog::getOpenFileName(QApplication::activeWindow(),
    "Load Calibration",
    m_LastCalibrationFilename.c_str(),
    "Calibration files *.cal");
  m_LastCalibrationFilename = filename.toStdString();

  mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality = this->GetSelectedCombinedModality();
  if (combinedModality.IsNull())
  {
    ui->calibrationLoadStateLabel->setText("Selected device is no USCombinedModality.");
    emit SignalNoLongerReadyForNextStep();
    return;
  }

  if (filename.isEmpty())
  {
    bool calibrated = this->UpdateCalibrationState();
    if (!calibrated) { emit SignalNoLongerReadyForNextStep(); }

    return;
  }

  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    MITK_WARN << "Cannot open file '" << filename.toStdString() << "' for reading.";
    ui->calibrationLoadStateLabel->setText("Cannot open file '" + filename + "' for reading.");
    emit SignalNoLongerReadyForNextStep();
    return;
  }

  QTextStream inStream(&file);
  m_LoadedCalibration = inStream.readAll().toStdString();
  if (m_LoadedCalibration.empty())
  {
    MITK_WARN << "Failed to load file. Unsupported format?";
    ui->calibrationLoadStateLabel->setText("Failed to load file. Unsupported format?");
    emit SignalNoLongerReadyForNextStep();
    return;
  }

  try
  {
    combinedModality->DeserializeCalibration(m_LoadedCalibration);
  }
  catch (const mitk::Exception& /*exception*/)
  {
    MITK_WARN << "Failed to deserialize calibration. Unsuppoerted format?";
    ui->calibrationLoadStateLabel->setText("Failed to deserialize calibration. Unsuppoerted format?");
    emit SignalNoLongerReadyForNextStep();
    return;
  }

  ui->calibrationLoadStateLabel->setText("Loaded calibration : " + filename);

  m_CombinedModality = combinedModality;

  emit SignalReadyForNextStep();
}

void QmitkUSNavigationStepCombinedModality::OnCombinedModalityCreateNewButtonClicked()
{
  this->SetCombinedModalityCreateWidgetEnabled(true);
}

void QmitkUSNavigationStepCombinedModality::OnCombinedModalityCreationExit()
{
  this->SetCombinedModalityCreateWidgetEnabled(false);
  mitk::DataNode::Pointer usNode = mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4"))->GetDataStorage()//GetDataStorage
    ->GetNamedNode("US Viewing Stream - Image 0");
  if (usNode.IsNotNull())
  {
    mitk::RenderingManager::GetInstance()->InitializeViews(//Reinit
      usNode->GetData()->GetTimeGeometry());//GetNode
  }
  else
  {
    MITK_WARN << "No reinit possible";
  }
}

void QmitkUSNavigationStepCombinedModality::OnCombinedModalityEditExit()
{
  this->SetCombinedModalityEditWidgetEnabled(false);
  ui->combinedModalityEditWidget->SetCombinedModality(0);
}

void QmitkUSNavigationStepCombinedModality::OnDeleteButtonClicked()
{
  mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality = this->GetSelectedCombinedModality();
  if (combinedModality.IsNotNull())
  {
    combinedModality->RemoveAllObservers();
    combinedModality->UnregisterOnService();
  }
}

void QmitkUSNavigationStepCombinedModality::OnCombinedModalityEditButtonClicked()
{
  ui->combinedModalityEditWidget->SetCombinedModality(m_CombinedModality);
  this->SetCombinedModalityEditWidgetEnabled(true);
}

void QmitkUSNavigationStepCombinedModality::OnActivateButtonClicked()
{
  mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality = this->GetSelectedCombinedModality();
  if (combinedModality.IsNotNull())
  {
    if (!combinedModality->GetUltrasoundDevice()->GetIsConnected()) { combinedModality->GetUltrasoundDevice()->Connect(); }
    if (!combinedModality->GetUltrasoundDevice()->GetIsActive()) { combinedModality->GetUltrasoundDevice()->Activate(); }
  }
}

void QmitkUSNavigationStepCombinedModality::OnDisconnectButtonClicked()
{
  mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality = this->GetSelectedCombinedModality();
  if (combinedModality.IsNotNull())
  {
    if (combinedModality->GetUltrasoundDevice()->GetIsActive()) { combinedModality->GetUltrasoundDevice()->Deactivate(); }
    if (combinedModality->GetUltrasoundDevice()->GetIsConnected()) { combinedModality->GetUltrasoundDevice()->Disconnect(); }
  }
}

bool QmitkUSNavigationStepCombinedModality::OnStartStep()
{
  return true;
}

bool QmitkUSNavigationStepCombinedModality::OnRestartStep()
{
  return true;
}

bool QmitkUSNavigationStepCombinedModality::OnFinishStep()
{
  mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality = this->GetSelectedCombinedModality();
  if (combinedModality.IsNotNull())
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    // make sure that the combined modality is in connected state before using it
    if (combinedModality->GetUltrasoundDevice()->GetDeviceState() < mitk::USDevice::State_Connected) { combinedModality->GetUltrasoundDevice()->Connect(); }
    if (combinedModality->GetUltrasoundDevice()->GetDeviceState() < mitk::USDevice::State_Activated) { combinedModality->GetUltrasoundDevice()->Activate(); }
    QApplication::restoreOverrideCursor();
  }

  emit SignalCombinedModalityChanged(combinedModality);

  this->CreateCombinedModalityResultAndSignalIt();

  return true;
}

bool QmitkUSNavigationStepCombinedModality::OnActivateStep()
{
  // make sure that device selection status is up-to-date
  this->OnDeviceSelectionChanged();

  return true;
}

void QmitkUSNavigationStepCombinedModality::OnUpdate()
{
}

QString QmitkUSNavigationStepCombinedModality::GetTitle()
{
  return "Selection of Combined Modality";
}

bool QmitkUSNavigationStepCombinedModality::GetIsRestartable()
{
  return false;
}

void QmitkUSNavigationStepCombinedModality::SetCombinedModalityCreateWidgetEnabled(bool enabled)
{
  ui->combinedModalityLabel->setVisible(!enabled);
  ui->combinedModalityListWidget->setVisible(!enabled);
  ui->combinedModalityCreateButton->setVisible(!enabled);
  ui->combinedModalityDeleteButton->setVisible(!enabled);
  ui->combinedModalitEditButton->setVisible(!enabled);
  ui->combinedModalityActivateButton->setVisible(!enabled);
  ui->combinedModalityDistconnectButton->setVisible(!enabled);
  ui->helpLabel->setVisible(!enabled);
  ui->calibrationGroupBox->setVisible(!enabled);
  ui->combinedModalityCreateWidget->setVisible(enabled);
}

void QmitkUSNavigationStepCombinedModality::SetCombinedModalityEditWidgetEnabled(bool enabled)
{
  ui->combinedModalityLabel->setVisible(!enabled);
  ui->combinedModalityListWidget->setVisible(!enabled);
  ui->combinedModalityCreateButton->setVisible(!enabled);
  ui->combinedModalityDeleteButton->setVisible(!enabled);
  ui->combinedModalitEditButton->setVisible(!enabled);
  ui->combinedModalityActivateButton->setVisible(!enabled);
  ui->combinedModalityDistconnectButton->setVisible(!enabled);
  ui->helpLabel->setVisible(!enabled);
  ui->calibrationGroupBox->setVisible(!enabled);
  ui->combinedModalityEditWidget->setVisible(enabled);
}

void QmitkUSNavigationStepCombinedModality::CreateCombinedModalityResultAndSignalIt()
{
  mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality = this->GetCombinedModality();
  mitk::USDevice::Pointer usDevice = combinedModality->GetUltrasoundDevice();

  // save identifiers and calibration to a result object
  mitk::DataNode::Pointer combinedModalityResult = mitk::DataNode::New();
  combinedModalityResult->SetName("CombinedModalityResult");
  combinedModalityResult->SetStringProperty("USNavigation::CombinedModality",
    std::string(combinedModality->GetUltrasoundDevice()->GetManufacturer() + ": " + combinedModality->GetUltrasoundDevice()->GetName()
    + " (" + combinedModality->GetUltrasoundDevice()->GetComment() + ")").c_str());
  combinedModalityResult->SetStringProperty("USNavigation::UltrasoundDevice",
    std::string(usDevice->GetManufacturer() + ": " + usDevice->GetName()
    + " (" + usDevice->GetComment() + ")").c_str());
  combinedModalityResult->SetStringProperty("USNavigation::TrackingDevice",
    combinedModality->GetNavigationDataSource()->GetName().c_str());
  combinedModalityResult->SetStringProperty("USNavigation::Calibration",
    combinedModality->SerializeCalibration().c_str());

  emit SignalIntermediateResult(combinedModalityResult);
}

bool QmitkUSNavigationStepCombinedModality::UpdateCalibrationState()
{
  if (m_CombinedModality.IsNull()) { return false; }

  bool calibrated = m_CombinedModality->GetContainsAtLeastOneCalibration();

  if (calibrated) { ui->calibrationLoadStateLabel->setText("Selected device contains at least one calibration."); }
  else { ui->calibrationLoadStateLabel->setText("Selected device is not calibrated."); }

  return calibrated;
}

mitk::AbstractUltrasoundTrackerDevice::Pointer QmitkUSNavigationStepCombinedModality::GetSelectedCombinedModality()
{
  // nothing more to do if no device is selected at the moment
  if (!ui->combinedModalityListWidget->GetIsServiceSelected()) { return 0; }

  mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality = ui->combinedModalityListWidget->GetSelectedService<mitk::AbstractUltrasoundTrackerDevice>();

  if (combinedModality.IsNull())
  {
    MITK_WARN << "Selected device is no USCombinedModality.";
  }

  return combinedModality;
}

void QmitkUSNavigationStepCombinedModality::SetCalibrationLoadedNecessary(bool necessary)
{
  m_CalibrationLoadedNecessary = necessary;
}

void QmitkUSNavigationStepCombinedModality::OnDevicePropertyChanged(const std::string& key, const std::string&)
{
  // property changes only matter if the navigation step is currently active
  // (being sensitive to them in other states may even be dangerous)
  if (this->GetNavigationStepState() < QmitkUSAbstractNavigationStep::State_Active) { return; }

  // calibration state could have changed if the depth was changed
  if (key == mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_DEPTH)
  {
    bool calibrated = this->UpdateCalibrationState();
    if (calibrated) { emit SignalReadyForNextStep(); }
    else { emit SignalNoLongerReadyForNextStep(); }
  }
}

void QmitkUSNavigationStepCombinedModality::UpdateTrackingToolNames()
{
  //check if everything is initialized
  if (m_CombinedModality.IsNull()) { return; }
  mitk::NavigationDataSource::Pointer navigationDataSource = m_CombinedModality->GetNavigationDataSource();
  if (navigationDataSource.IsNull()) { return; }
  if (GetDataStorage(false).IsNull()) { return; }

  // get the settings node
  mitk::DataNode::Pointer settingsNode = this->GetNamedDerivedNode(DATANAME_SETTINGS, DATANAME_BASENODE);
  std::string needleNames;

  itk::ProcessObject::DataObjectPointerArray outputs = navigationDataSource->GetOutputs();
  for (itk::ProcessObject::DataObjectPointerArray::iterator it = outputs.begin(); it != outputs.end(); ++it)
  {
    needleNames += std::string((static_cast<mitk::NavigationData*>(it->GetPointer()))->GetName()) + ";";
  }

  // change the settings node only if the settings changed
  std::string oldProperty;
  if (!settingsNode->GetStringProperty("settings.needle-names", oldProperty)
    || oldProperty != needleNames
    || !settingsNode->GetStringProperty("settings.reference-names", oldProperty)
    || oldProperty != needleNames)
  {
    settingsNode->SetStringProperty("settings.needle-names", needleNames.c_str());
    settingsNode->SetStringProperty("settings.reference-names", needleNames.c_str());

    emit SignalSettingsNodeChanged(settingsNode);
  }
}
