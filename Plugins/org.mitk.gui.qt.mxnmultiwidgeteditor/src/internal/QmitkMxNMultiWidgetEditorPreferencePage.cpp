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
#include <berryIEditorRegistry.h>
#include <berryIWorkbench.h>
#include <berryObjectString.h>
#include <berryObjectStringMap.h>

#include <QLabel>
#include <QVBoxLayout>

QmitkMxNMultiWidgetEditorPreferencePage::QmitkMxNMultiWidgetEditorPreferencePage()
  : m_Ui(std::make_unique<Ui::QmitkMxNMultiWidgetEditorPreferencePage>())
  , m_EditorId(QmitkMxNMultiWidgetEditor::EDITOR_ID)
{
  // nothing here
}

void QmitkMxNMultiWidgetEditorPreferencePage::SetInitializationData(const berry::IConfigurationElement::Pointer& config,
                                                                    const QString& propertyName,
                                                                    const berry::Object::Pointer& data)
{
  m_PageContext = QStringLiteral("Preference page '%1' declared by plugin '%2'")
    .arg(config->GetAttribute("id"), config->GetContributor()->GetName());

  if (data.IsNull())
  {
    return;
  }

  const auto* parameters = dynamic_cast<const berry::ObjectStringMap*>(data.GetPointer());
  if (nullptr == parameters)
  {
    // The colon form 'class="Cls:data"' carries no parameter names.
    const auto* value = dynamic_cast<const berry::ObjectString*>(data.GetPointer());
    this->RecordConfigurationError(propertyName,
      QStringLiteral("'%1' carries unnamed data '%2'; declare <parameter> children of a <class> element instead.")
        .arg(config->GetAttribute(propertyName), nullptr != value ? QString(*value) : QString()));
    return;
  }

  for (auto it = parameters->cbegin(); it != parameters->cend(); ++it)
  {
    if (it.key() != QLatin1String("editorId"))
    {
      this->RecordConfigurationError(it.key(),
        QStringLiteral("unknown parameter (value '%1'); expected 'editorId'.").arg(it.value()));
      continue;
    }
    if (it.value().trimmed().isEmpty())
    {
      this->RecordConfigurationError(QStringLiteral("editorId"), QStringLiteral("must not be empty."));
      continue;
    }
    m_EditorId = it.value().trimmed();
  }
}

void QmitkMxNMultiWidgetEditorPreferencePage::RecordConfigurationError(const QString& parameter,
                                                                       const QString& problem)
{
  const auto message = QStringLiteral("%1, parameter '%2': %3").arg(m_PageContext, parameter, problem);
  MITK_ERROR << message.toStdString();
  m_ConfigurationErrors.append(message);
}

void QmitkMxNMultiWidgetEditorPreferencePage::ShowConfigurationErrors(QWidget* parent)
{
  auto* label = new QLabel(parent);
  label->setTextFormat(Qt::PlainText);
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignCenter);
  label->setTextInteractionFlags(Qt::TextSelectableByMouse);
  label->setText(QStringLiteral("These preferences cannot be shown because the page's declaration is invalid.\n\n")
                 + m_ConfigurationErrors.join(QStringLiteral("\n")));
  parent->layout()->addWidget(label);
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

void QmitkMxNMultiWidgetEditorPreferencePage::Init(berry::IWorkbench::Pointer workbench)
{
  // Checked against the passed workbench rather than PlatformUI::GetWorkbench(),
  // which throws when no workbench exists.
  if (m_ConfigurationErrors.isEmpty() && workbench.IsNotNull() &&
      workbench->GetEditorRegistry()->FindEditor(m_EditorId).IsNull())
  {
    this->RecordConfigurationError(QStringLiteral("editorId"),
                                   QStringLiteral("'%1' is not a registered editor ID.").arg(m_EditorId));
  }
}

void QmitkMxNMultiWidgetEditorPreferencePage::CreateQtControl(QWidget* parent)
{
  m_MainControl = new QWidget(parent);

  // Without setupUi the Ui widgets do not exist, and the declared node is not
  // known to be the right one.
  if (!m_ConfigurationErrors.isEmpty())
  {
    new QVBoxLayout(m_MainControl);
    this->ShowConfigurationErrors(m_MainControl);
    return;
  }

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
  if (!m_ConfigurationErrors.isEmpty())
  {
    return true;
  }

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
  if (!m_ConfigurationErrors.isEmpty())
  {
    return;
  }

  auto* prefs = this->GetPreferences();

  m_Ui->m_ShowLevelWindowReadout->setChecked(prefs->GetBool("Show level/window readout", true));
  m_Ui->m_ExpandedNavigator->setChecked(prefs->GetBool("Expanded navigator", false));
  m_Ui->m_PACSLikeMouseMode->setChecked(prefs->GetBool("PACS like mouse interaction", false));

  m_Ui->m_CrosshairGapSize->setValue(prefs->GetInt("crosshair gap size", 32));
}

void QmitkMxNMultiWidgetEditorPreferencePage::ResetPreferencesAndGUI()
{
  if (!m_ConfigurationErrors.isEmpty())
  {
    return;
  }

  auto prefs = this->GetPreferences();
  prefs->Clear();

  this->Update();
  this->PerformOk();
}
