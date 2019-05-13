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
#include "QmitkUSNavigationCombinedSettingsWidget.h"
#include "ui_QmitkUSNavigationCombinedSettingsWidget.h"

#include "mitkCommon.h"
#include "mitkDataNode.h"

#include <ctkDirectoryButton.h>

#include <QSettings>

QmitkUSNavigationCombinedSettingsWidget::QmitkUSNavigationCombinedSettingsWidget(QWidget *parent) :
QmitkUSNavigationAbstractSettingsWidget(parent),
  ui(new Ui::QmitkUSNavigationCombinedSettingsWidget)
{
  ui->setupUi(this);
  ui->experimentResultsPathButton->setOptions(ctkDirectoryButton::ShowDirsOnly);

  connect( ui->applicationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnApplicationChanged(int)) );
}

QmitkUSNavigationCombinedSettingsWidget::~QmitkUSNavigationCombinedSettingsWidget()
{
  delete ui;
}

void QmitkUSNavigationCombinedSettingsWidget::OnSetSettingsNode(itk::SmartPointer<mitk::DataNode> settingsNode, bool overwriteValues)
{
  if ( overwriteValues )
  {
    settingsNode->SetStringProperty("settings.application", ui->applicationComboBox->currentText().toStdString().c_str());
    settingsNode->SetStringProperty("settings.interaction-concept", this->InteractionNameToFile(ui->interactionConceptComboBox->currentText()).toStdString().c_str());
    settingsNode->SetBoolProperty("settings.experiment-mode", ui->experimentModeCheckBox->isChecked());
    settingsNode->SetStringProperty("settings.experiment-results-directory", ui->experimentResultsPathButton->directory().toStdString().c_str());

    settingsNode->SetFloatProperty("settings.security-distance", ui->securityDistanceSpinBox->value());
    settingsNode->SetIntProperty("settings.number-of-targets", ui->numberOfTargetsSpinBox->value());
    settingsNode->SetBoolProperty("settings.use-planning-step", ui->planningStepUsageCheckBox->isChecked());

    settingsNode->SetStringProperty("settings.needle-name-selected", ui->needleNameComboBox->currentText().toStdString().c_str());
    settingsNode->SetStringProperty("settings.reference-name-selected", ui->referenceNameComboBox->currentText().toStdString().c_str());
  }
  else
  {
    std::string stringProperty;
    float floatProperty;
    bool boolProperty;

    // load state of application setting combo box
    if ( ! settingsNode->GetStringProperty("settings.application", stringProperty) ) { stringProperty = ""; }
    int lastIndex = ui->applicationComboBox->findText(QString::fromStdString(stringProperty));
    if (lastIndex == -1) { lastIndex = 0; }
    ui->applicationComboBox->setCurrentIndex(lastIndex);
    this->OnApplicationChanged(lastIndex);

    // load state of interaction concept setting combo box
    if ( ! settingsNode->GetStringProperty("settings.interaction-concept", stringProperty) ) { stringProperty = ""; }
    lastIndex = ui->interactionConceptComboBox->findText(QString::fromStdString(stringProperty));
    if (lastIndex == -1) { lastIndex = 0; }
    ui->interactionConceptComboBox->setCurrentIndex(lastIndex);

    if ( ! settingsNode->GetBoolProperty("settings.experiment-mode", boolProperty) ) { boolProperty = false; }
    ui->experimentModeCheckBox->setChecked(boolProperty);

    if ( ! settingsNode->GetStringProperty("settings.experiment-results-directory", stringProperty) ) { stringProperty = ""; }
    ui->experimentResultsPathButton->setDirectory(QString::fromStdString(stringProperty));

    if ( ! settingsNode->GetFloatProperty("settings.security-distance", floatProperty) ) { floatProperty = 2.0; }
    ui->securityDistanceSpinBox->setValue(floatProperty);

    if ( ! settingsNode->GetFloatProperty("settings.number-of-targets", floatProperty) ) { floatProperty = 3; }
    ui->numberOfTargetsSpinBox->setValue(static_cast<int>(floatProperty));

    if ( ! settingsNode->GetBoolProperty("settings.use-planning-step", boolProperty) ) { boolProperty = true; }
    ui->planningStepUsageCheckBox->setChecked(boolProperty);

    if ( ! settingsNode->GetStringProperty("settings.needle-names", stringProperty) ) { stringProperty = ""; }
    QString needleNames = QString::fromStdString(stringProperty);

    ui->needleNameComboBox->clear();
    ui->needleNameComboBox->addItems(needleNames.split(";", QString::SkipEmptyParts));

    if ( ! settingsNode->GetStringProperty("settings.needle-name-selected", stringProperty) ) { stringProperty = ""; }
    int index = ui->needleNameComboBox->findText(QString::fromStdString(stringProperty));
    if (index == -1)
    {
      ui->needleNameComboBox->addItem(QString::fromStdString(stringProperty));
      ui->needleNameComboBox->setCurrentIndex(ui->needleNameComboBox->count()-1);
    }
    else
    {
      ui->needleNameComboBox->setCurrentIndex(index);
    }

    if ( ! settingsNode->GetStringProperty("settings.reference-names", stringProperty) ) { stringProperty = ""; }
    QString referenceNames = QString::fromStdString(stringProperty);

    ui->referenceNameComboBox->clear();
    ui->referenceNameComboBox->addItems(referenceNames.split(";", QString::SkipEmptyParts));

    if ( ! settingsNode->GetStringProperty("settings.reference-name-selected", stringProperty) ) { stringProperty = ""; }
    index = ui->referenceNameComboBox->findText(QString::fromStdString(stringProperty));
    if (index == -1)
    {
      ui->referenceNameComboBox->addItem(QString::fromStdString(stringProperty));
      ui->referenceNameComboBox->setCurrentIndex(ui->referenceNameComboBox->count()-1);
    }
    else
    {
      ui->referenceNameComboBox->setCurrentIndex(index);
    }
  }
}

void QmitkUSNavigationCombinedSettingsWidget::OnSaveProcessing()
{
  mitk::DataNode::Pointer settingsNode = this->GetSettingsNode();

  QSettings settings;
  settings.beginGroup("ultrasound-navigation");

  settings.setValue("application", ui->applicationComboBox->currentText());
  settingsNode->SetStringProperty("settings.application", ui->applicationComboBox->currentText().toStdString().c_str());

  settings.setValue("interaction-concept", ui->interactionConceptComboBox->currentText());
  settingsNode->SetStringProperty("settings.interaction-concept", this->InteractionNameToFile(ui->interactionConceptComboBox->currentText()).toStdString().c_str());

  settings.setValue("experiment-mode", ui->experimentModeCheckBox->isChecked());
  settingsNode->SetBoolProperty("settings.experiment-mode", ui->experimentModeCheckBox->isChecked());

  settings.setValue("experiment-results-directory", ui->experimentResultsPathButton->directory());
  settingsNode->SetStringProperty("settings.experiment-results-directory", ui->experimentResultsPathButton->directory().toStdString().c_str());

  settings.setValue("security-distance", ui->securityDistanceSpinBox->value());
  settingsNode->SetFloatProperty("settings.security-distance", ui->securityDistanceSpinBox->value());

  settings.setValue("number-of-targets", ui->numberOfTargetsSpinBox->value());
  settingsNode->SetIntProperty("settings.number-of-targets", ui->numberOfTargetsSpinBox->value());

  settings.setValue("use-planning-step", ui->planningStepUsageCheckBox->isChecked());
  settingsNode->SetBoolProperty("settings.use-planning-step", ui->planningStepUsageCheckBox->isChecked());

  settings.setValue("needle-name-selected", ui->needleNameComboBox->currentText());
  settingsNode->SetStringProperty("settings.needle-name-selected", ui->needleNameComboBox->currentText().toStdString().c_str());

  settings.setValue("reference-name-selected", ui->referenceNameComboBox->currentText());
  settingsNode->SetStringProperty("settings.reference-name-selected", ui->referenceNameComboBox->currentText().toStdString().c_str());

  settings.endGroup();
}

void QmitkUSNavigationCombinedSettingsWidget::OnLoadSettingsProcessing()
{
  QSettings settings;
  settings.beginGroup("ultrasound-navigation");

  // load state of application setting combo box
  int lastIndex = ui->applicationComboBox->findText(settings.value("application").toString());
  if (lastIndex == -1) { lastIndex = 0; }
  ui->applicationComboBox->setCurrentIndex(lastIndex);
  this->OnApplicationChanged(lastIndex);

  // load state of interaction concept setting combo box
  lastIndex = ui->interactionConceptComboBox->findText(settings.value("interaction-concept").toString());
  if (lastIndex == -1) { lastIndex = 0; }
  ui->interactionConceptComboBox->setCurrentIndex(lastIndex);

  ui->experimentModeCheckBox->setChecked(settings.value("experiment-mode").toBool());
  ui->experimentResultsPathButton->setDirectory(settings.value("experiment-results-directory").toString());

  ui->securityDistanceSpinBox->setValue(settings.value("security-distance", 2.0).toDouble());
  ui->numberOfTargetsSpinBox->setValue(settings.value("number-of-targets", 3).toInt());

  ui->planningStepUsageCheckBox->setChecked(settings.value("use-planning-step", true).toBool());

  ui->needleNameComboBox->clear();
  ui->needleNameComboBox->addItems(settings.value("needle-names").toStringList());

  QString selectedName = settings.value("needle-name-selected").toString();
  int index = ui->needleNameComboBox->findText(selectedName);
  if (index == -1)
  {
    ui->needleNameComboBox->addItem(selectedName);
    ui->needleNameComboBox->setCurrentIndex(ui->needleNameComboBox->count()-1);
  }
  else
  {
    ui->needleNameComboBox->setCurrentIndex(index);
  }

  ui->referenceNameComboBox->clear();
  ui->referenceNameComboBox->addItems(settings.value("reference-names").toStringList());

  selectedName = settings.value("reference-name-selected").toString();
  index = ui->referenceNameComboBox->findText(selectedName);
  if (index == -1)
  {
    ui->referenceNameComboBox->addItem(selectedName);
    ui->referenceNameComboBox->setCurrentIndex(ui->referenceNameComboBox->count()-1);
  }
  else
  {
    ui->referenceNameComboBox->setCurrentIndex(index);
  }

  settings.endGroup();
}

void QmitkUSNavigationCombinedSettingsWidget::OnApplicationChanged(int index)
{
  if (index == 0)
  {
    ui->punctuationGroupBox->show();
    ui->markerPlacementGroupBox->hide();
  }
  else if (index == 1)
  {
    ui->punctuationGroupBox->hide();
    ui->markerPlacementGroupBox->show();
  }
}

QString QmitkUSNavigationCombinedSettingsWidget::InteractionNameToFile(const QString& name) const
{
  if (name == tr("Click, Move and Click"))
  {
    return QString("USZoneInteractions.xml");
  }
  else if (name == tr("Hold, Move and Release"))
  {
    return QString("USZoneInteractionsHold.xml");
  }
  else
  {
    return QString();
  }
}
