/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNMultiWidgetEditorPreferencePage.h"
#include <ui_QmitkMxNMultiWidgetEditorPreferencePage.h>
#include "QmitkMxNMultiWidgetEditor.h"

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>
#include <mitkLog.h>

#include <berryIConfigurationElement.h>
#include <berryIContributor.h>
#include <berryObjectStringMap.h>

QmitkMxNMultiWidgetEditorPreferencePage::QmitkMxNMultiWidgetEditorPreferencePage()
  : m_Ui(std::make_unique<Ui::QmitkMxNMultiWidgetEditorPreferencePage>())
  , m_EditorId(QmitkMxNMultiWidgetEditor::EDITOR_ID)
{
  // nothing here
}

void QmitkMxNMultiWidgetEditorPreferencePage::SetInitializationData(const berry::IConfigurationElement::Pointer& config,
                                                                    const QString& /*propertyName*/,
                                                                    const berry::Object::Pointer& data)
{
  const auto* parameters = dynamic_cast<const berry::ObjectStringMap*>(data.GetPointer());
  if (nullptr == parameters)
  {
    return;
  }

  const auto context = QStringLiteral("Preference page '%1' declared by plugin '%2'")
    .arg(config->GetAttribute("id"), config->GetContributor()->GetName());

  for (auto it = parameters->cbegin(); it != parameters->cend(); ++it)
  {
    if (it.key() != QLatin1String("editorId"))
    {
      MITK_WARN << context.toStdString() << ": ignoring unknown parameter '" << it.key().toStdString() << "'.";
      continue;
    }
    if (it.value().trimmed().isEmpty())
    {
      MITK_WARN << context.toStdString() << ": empty 'editorId'; editing the preferences of '"
                << QmitkMxNMultiWidgetEditor::EDITOR_ID.toStdString() << "'.";
      continue;
    }
    m_EditorId = it.value().trimmed();
  }
}

mitk::IPreferences* QmitkMxNMultiWidgetEditorPreferencePage::GetPreferences() const
{
  // An editor reads the node named after its own editor ID.
  auto* preferencesService = mitk::CoreServices::GetPreferencesService();
  return preferencesService->GetSystemPreferences()->Node(m_EditorId.toStdString());
}

QmitkMxNMultiWidgetEditorPreferencePage::~QmitkMxNMultiWidgetEditorPreferencePage()
{
  //nothing here
}

void QmitkMxNMultiWidgetEditorPreferencePage::Init(berry::IWorkbench::Pointer)
{
  // nothing here
}

void QmitkMxNMultiWidgetEditorPreferencePage::CreateQtControl(QWidget* parent)
{
  m_MainControl = new QWidget(parent);

  m_Ui->setupUi(m_MainControl);

  connect(m_Ui->m_ResetButton, SIGNAL(clicked()), SLOT(ResetPreferencesAndGUI()));

  Update();
}

QWidget* QmitkMxNMultiWidgetEditorPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkMxNMultiWidgetEditorPreferencePage::PerformOk()
{
  auto* prefs = this->GetPreferences();

  prefs->PutBool("Show level/window readout", m_Ui->m_ShowLevelWindowReadout->isChecked());
  prefs->PutBool("Expanded navigator", m_Ui->m_ExpandedNavigator->isChecked());
  prefs->PutBool("PACS like mouse interaction", m_Ui->m_PACSLikeMouseMode->isChecked());

  prefs->PutInt("crosshair gap size", m_Ui->m_CrosshairGapSize->value());

  return true;
}

void QmitkMxNMultiWidgetEditorPreferencePage::PerformCancel()
{
  // nothing here
}

void QmitkMxNMultiWidgetEditorPreferencePage::Update()
{
  auto* prefs = this->GetPreferences();

  m_Ui->m_ShowLevelWindowReadout->setChecked(prefs->GetBool("Show level/window readout", true));
  m_Ui->m_ExpandedNavigator->setChecked(prefs->GetBool("Expanded navigator", false));
  m_Ui->m_PACSLikeMouseMode->setChecked(prefs->GetBool("PACS like mouse interaction", false));

  m_Ui->m_CrosshairGapSize->setValue(prefs->GetInt("crosshair gap size", 32));
}

void QmitkMxNMultiWidgetEditorPreferencePage::ResetPreferencesAndGUI()
{
  auto prefs = this->GetPreferences();
  prefs->Clear();

  this->Update();
  this->PerformOk();
}
