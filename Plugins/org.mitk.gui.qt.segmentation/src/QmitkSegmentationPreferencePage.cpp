/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegmentationPreferencePage.h"

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include <mitkBaseApplication.h>

#include <QFileDialog>

#include <ui_QmitkSegmentationPreferencePageControls.h>

QmitkSegmentationPreferencePage::QmitkSegmentationPreferencePage()
  : m_Ui(new Ui::QmitkSegmentationPreferencePageControls),
    m_Control(nullptr),
    m_Initializing(false)
{
}

QmitkSegmentationPreferencePage::~QmitkSegmentationPreferencePage()
{
}

void QmitkSegmentationPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkSegmentationPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Initializing = true;
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  m_SegmentationPreferencesNode = prefService->GetSystemPreferences()->Node("/org.mitk.views.segmentation");

  m_Control = new QWidget(parent);
  m_Ui->setupUi(m_Control);

  connect(m_Ui->smoothingCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnSmoothingCheckboxChecked(int)));
  connect(m_Ui->labelSetPresetToolButton, SIGNAL(clicked()), this, SLOT(OnLabelSetPresetButtonClicked()));
  connect(m_Ui->suggestionsToolButton, SIGNAL(clicked()), this, SLOT(OnSuggestionsButtonClicked()));

  this->Update();
  m_Initializing = false;
}

QWidget* QmitkSegmentationPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkSegmentationPreferencePage::PerformOk()
{
  m_SegmentationPreferencesNode->PutBool("slim view", m_Ui->slimViewCheckBox->isChecked());
  m_SegmentationPreferencesNode->PutBool("draw outline", m_Ui->outlineRadioButton->isChecked());
  m_SegmentationPreferencesNode->PutBool("selection mode", m_Ui->selectionModeCheckBox->isChecked());
  m_SegmentationPreferencesNode->PutBool("smoothing hint", m_Ui->smoothingCheckBox->isChecked());
  m_SegmentationPreferencesNode->PutDouble("smoothing value", m_Ui->smoothingSpinBox->value());
  m_SegmentationPreferencesNode->PutDouble("decimation rate", m_Ui->decimationSpinBox->value());
  m_SegmentationPreferencesNode->PutDouble("closing ratio", m_Ui->closingSpinBox->value());
  m_SegmentationPreferencesNode->Put("label set preset", m_Ui->labelSetPresetLineEdit->text());
  m_SegmentationPreferencesNode->PutBool("default label naming", m_Ui->defaultNameRadioButton->isChecked());
  m_SegmentationPreferencesNode->Put("label suggestions", m_Ui->suggestionsLineEdit->text());
  m_SegmentationPreferencesNode->PutBool("replace standard suggestions", m_Ui->replaceStandardSuggestionsCheckBox->isChecked());
  m_SegmentationPreferencesNode->PutBool("suggest once", m_Ui->suggestOnceCheckBox->isChecked());
  return true;
}

void QmitkSegmentationPreferencePage::PerformCancel()
{
}

void QmitkSegmentationPreferencePage::Update()
{
  m_Ui->slimViewCheckBox->setChecked(m_SegmentationPreferencesNode->GetBool("slim view", false));

  if (m_SegmentationPreferencesNode->GetBool("draw outline", true))
  {
    m_Ui->outlineRadioButton->setChecked(true);
  }
  else
  {
    m_Ui->overlayRadioButton->setChecked(true);
  }

  m_Ui->selectionModeCheckBox->setChecked(m_SegmentationPreferencesNode->GetBool("selection mode", false));

  if (m_SegmentationPreferencesNode->GetBool("smoothing hint", true))
  {
    m_Ui->smoothingCheckBox->setChecked(true);
    m_Ui->smoothingSpinBox->setDisabled(true);
  }
  else
  {
    m_Ui->smoothingCheckBox->setChecked(false);
    m_Ui->smoothingSpinBox->setEnabled(true);
  }

  m_Ui->smoothingSpinBox->setValue(m_SegmentationPreferencesNode->GetDouble("smoothing value", 1.0));
  m_Ui->decimationSpinBox->setValue(m_SegmentationPreferencesNode->GetDouble("decimation rate", 0.5));
  m_Ui->closingSpinBox->setValue(m_SegmentationPreferencesNode->GetDouble("closing ratio", 0.0));

  auto labelSetPreset = mitk::BaseApplication::instance().config().getString(mitk::BaseApplication::ARG_SEGMENTATION_LABELSET_PRESET.toStdString(), "");
  bool isOverriddenByCmdLineArg = !labelSetPreset.empty();

  if (!isOverriddenByCmdLineArg)
    labelSetPreset = m_SegmentationPreferencesNode->Get("label set preset", "").toStdString();

  m_Ui->labelSetPresetLineEdit->setDisabled(isOverriddenByCmdLineArg);
  m_Ui->labelSetPresetToolButton->setDisabled(isOverriddenByCmdLineArg);
  m_Ui->labelSetPresetCmdLineArgLabel->setVisible(isOverriddenByCmdLineArg);

  m_Ui->labelSetPresetLineEdit->setText(QString::fromStdString(labelSetPreset));

  if (m_SegmentationPreferencesNode->GetBool("default label naming", true))
  {
    m_Ui->defaultNameRadioButton->setChecked(true);
  }
  else
  {
    m_Ui->askForNameRadioButton->setChecked(true);
  }

  auto labelSuggestions = mitk::BaseApplication::instance().config().getString(mitk::BaseApplication::ARG_SEGMENTATION_LABEL_SUGGESTIONS.toStdString(), "");
  isOverriddenByCmdLineArg = !labelSuggestions.empty();

  if (!isOverriddenByCmdLineArg)
    labelSuggestions = m_SegmentationPreferencesNode->Get("label suggestions", "").toStdString();

  m_Ui->suggestionsLineEdit->setDisabled(isOverriddenByCmdLineArg);
  m_Ui->suggestionsToolButton->setDisabled(isOverriddenByCmdLineArg);
  m_Ui->suggestionsCmdLineArgLabel->setVisible(isOverriddenByCmdLineArg);

  m_Ui->suggestionsLineEdit->setText(QString::fromStdString(labelSuggestions));

  m_Ui->replaceStandardSuggestionsCheckBox->setChecked(m_SegmentationPreferencesNode->GetBool("replace standard suggestions", true));
  m_Ui->suggestOnceCheckBox->setChecked(m_SegmentationPreferencesNode->GetBool("suggest once", true));
}

void QmitkSegmentationPreferencePage::OnSmoothingCheckboxChecked(int state)
{
  if (state != Qt::Unchecked)
    m_Ui->smoothingSpinBox->setDisabled(true);
  else
    m_Ui->smoothingSpinBox->setEnabled(true);
}

void QmitkSegmentationPreferencePage::OnLabelSetPresetButtonClicked()
{
  const auto filename = QFileDialog::getOpenFileName(m_Control, QStringLiteral("Load Label Set Preset"), QString(), QStringLiteral("Label set preset (*.lsetp)"));

  if (!filename.isEmpty())
    m_Ui->labelSetPresetLineEdit->setText(filename);
}

void QmitkSegmentationPreferencePage::OnSuggestionsButtonClicked()
{
  const auto filename = QFileDialog::getOpenFileName(m_Control, QStringLiteral("Load Label Suggestions"), QString(), QStringLiteral("Label suggestions (*.json)"));

  if (!filename.isEmpty())
    m_Ui->suggestionsLineEdit->setText(filename);
}
