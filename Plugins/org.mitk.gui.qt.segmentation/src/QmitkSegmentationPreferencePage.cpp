/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegmentationPreferencePage.h"

#include <mitkBaseApplication.h>
#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <QFileDialog>

#include <ui_QmitkSegmentationPreferencePageControls.h>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  }
}

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

  m_Control = new QWidget(parent);
  m_Ui->setupUi(m_Control);

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
  auto* prefs = GetPreferences();

  prefs->PutBool("compact view", m_Ui->compactViewCheckBox->isChecked());
  prefs->PutBool("draw outline", m_Ui->outlineRadioButton->isChecked());
  prefs->PutBool("selection mode", m_Ui->selectionModeCheckBox->isChecked());
  prefs->Put("label set preset", m_Ui->labelSetPresetLineEdit->text().toStdString());
  prefs->PutBool("default label naming", m_Ui->defaultNameRadioButton->isChecked());
  prefs->Put("label suggestions", m_Ui->suggestionsLineEdit->text().toStdString());
  prefs->PutBool("replace standard suggestions", m_Ui->replaceStandardSuggestionsCheckBox->isChecked());
  prefs->PutBool("suggest once", m_Ui->suggestOnceCheckBox->isChecked());
  return true;
}

void QmitkSegmentationPreferencePage::PerformCancel()
{
}

void QmitkSegmentationPreferencePage::Update()
{
  auto* prefs = GetPreferences();

  m_Ui->compactViewCheckBox->setChecked(prefs->GetBool("compact view", false));

  if (prefs->GetBool("draw outline", true))
  {
    m_Ui->outlineRadioButton->setChecked(true);
  }
  else
  {
    m_Ui->overlayRadioButton->setChecked(true);
  }

  m_Ui->selectionModeCheckBox->setChecked(prefs->GetBool("selection mode", false));

  auto labelSetPreset = mitk::BaseApplication::instance().config().getString(mitk::BaseApplication::ARG_SEGMENTATION_LABELSET_PRESET.toStdString(), "");
  bool isOverriddenByCmdLineArg = !labelSetPreset.empty();

  if (!isOverriddenByCmdLineArg)
    labelSetPreset = prefs->Get("label set preset", "");

  m_Ui->labelSetPresetLineEdit->setDisabled(isOverriddenByCmdLineArg);
  m_Ui->labelSetPresetToolButton->setDisabled(isOverriddenByCmdLineArg);
  m_Ui->labelSetPresetCmdLineArgLabel->setVisible(isOverriddenByCmdLineArg);

  m_Ui->labelSetPresetLineEdit->setText(QString::fromStdString(labelSetPreset));

  if (prefs->GetBool("default label naming", true))
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
    labelSuggestions = prefs->Get("label suggestions", "");

  m_Ui->defaultNameRadioButton->setDisabled(isOverriddenByCmdLineArg);
  m_Ui->askForNameRadioButton->setDisabled(isOverriddenByCmdLineArg);
  m_Ui->suggestionsLineEdit->setDisabled(isOverriddenByCmdLineArg);
  m_Ui->suggestionsToolButton->setDisabled(isOverriddenByCmdLineArg);
  m_Ui->suggestionsCmdLineArgLabel->setVisible(isOverriddenByCmdLineArg);

  m_Ui->suggestionsLineEdit->setText(QString::fromStdString(labelSuggestions));

  m_Ui->replaceStandardSuggestionsCheckBox->setChecked(prefs->GetBool("replace standard suggestions", true));
  m_Ui->suggestOnceCheckBox->setChecked(prefs->GetBool("suggest once", true));
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
