/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNMultiWidgetEditor.h"

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>
#include <mitkLog.h>

#include <berryIContributor.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchPartConstants.h>
#include <berryObjectString.h>
#include <berryObjectStringMap.h>
#include <berryPlatform.h>
#include <berryUIException.h>

#include <ctkPlugin.h>

// mitk qt widgets module
#include <QmitkMxNMultiWidget.h>
#include <QmitkInteractionSchemeToolBar.h>

// qt
#include <QHBoxLayout>
#include <QLabel>

// c++
#include <optional>

const QString QmitkMxNMultiWidgetEditor::EDITOR_ID = "org.mitk.editors.mxnmultiwidget";

struct QmitkMxNMultiWidgetEditor::Impl final
{
  Impl();
  ~Impl() = default;

  QmitkInteractionSchemeToolBar* m_InteractionSchemeToolBar;

  // The scheme is editor-wide state the layout editor's toggle reads back, so
  // the two cannot disagree about which mode is live.
  mitk::InteractionSchemeSwitcher::InteractionScheme m_InteractionScheme;

  /** Empty until the preferences have been read for the first time. */
  std::optional<bool> m_PACSInteraction;

  // Read from the declaration in SetInitializationData.
  QString m_EditorId;
  QString m_DeclaringPlugin;
  QString m_WidgetName;
  QmitkMxNLayoutLocks m_LayoutLocks;
  std::optional<nlohmann::json> m_InitialLayout;

  /** Non-empty puts the editor into its error state: no multi widget. */
  QStringList m_ConfigurationErrors;
};

QmitkMxNMultiWidgetEditor::Impl::Impl()
  : m_InteractionSchemeToolBar(nullptr)
  , m_InteractionScheme(mitk::InteractionSchemeSwitcher::MITKStandard)
{
  // nothing here
}

//////////////////////////////////////////////////////////////////////////
// QmitkMxNMultiWidgetEditor
//////////////////////////////////////////////////////////////////////////
QmitkMxNMultiWidgetEditor::QmitkMxNMultiWidgetEditor()
  : QmitkAbstractMultiWidgetEditor()
  , m_Impl(std::make_unique<Impl>())
{
  // nothing here
}

QmitkMxNMultiWidgetEditor::~QmitkMxNMultiWidgetEditor()
{
  GetSite()->GetPage()->RemovePartListener(this);
}

berry::IPartListener::Events::Types QmitkMxNMultiWidgetEditor::GetPartEventTypes() const
{
  // Only OPENED: the other part events serve the built-in render-window
  // menu, which this editor never shows.
  return Events::OPENED;
}

void QmitkMxNMultiWidgetEditor::SetInitializationData(const berry::IConfigurationElement::Pointer& config,
                                                      const QString& propertyName,
                                                      const berry::Object::Pointer& data)
{
  QmitkAbstractMultiWidgetEditor::SetInitializationData(config, propertyName, data);

  // The site, and with it the part ID, is not set yet; the declaring element
  // carries the editor ID.
  m_Impl->m_EditorId = config->GetAttribute("id");
  m_Impl->m_DeclaringPlugin = config->GetContributor()->GetName();

  // The editor ID as default makes widget names unique without a naming rule.
  m_Impl->m_WidgetName = m_Impl->m_EditorId;
  bool widgetNameDeclared = false;

  QHash<QString, QString> parameters;
  if (auto parameterMap = dynamic_cast<const berry::ObjectStringMap*>(data.GetPointer()))
  {
    parameters = *parameterMap;
  }
  else if (data.IsNotNull())
  {
    // The colon form 'class="Cls:data"' carries no parameter names.
    const auto value = dynamic_cast<const berry::ObjectString*>(data.GetPointer());
    this->RecordConfigurationError(propertyName,
      QStringLiteral("'%1' carries unnamed data '%2'; declare <parameter> children of a <class> element instead.")
        .arg(config->GetAttribute(propertyName), nullptr != value ? QString(*value) : QString()));
  }

  for (auto it = parameters.cbegin(); it != parameters.cend(); ++it)
  {
    const auto& name = it.key();
    const auto& value = it.value();

    if (name == QLatin1String("widgetName"))
    {
      m_Impl->m_WidgetName = value;
      widgetNameDeclared = true;
    }
    else if (name == QLatin1String("layoutLock"))
    {
      try
      {
        m_Impl->m_LayoutLocks = QmitkMxNParseLayoutLocks(value);
      }
      catch (const mitk::Exception& e)
      {
        this->RecordConfigurationError(name, QString::fromStdString(e.GetDescription()));
      }
    }
    else if (name == QLatin1String("initialLayout"))
    {
      const auto plugin = berry::Platform::GetPlugin(m_Impl->m_DeclaringPlugin);
      const auto content = nullptr != plugin ? plugin->getResource(value) : QByteArray();
      if (content.isEmpty())
      {
        this->RecordConfigurationError(name,
          QStringLiteral("resource '%1' is missing or empty in plugin '%2'.").arg(value, m_Impl->m_DeclaringPlugin));
        continue;
      }
      try
      {
        m_Impl->m_InitialLayout = nlohmann::json::parse(content.cbegin(), content.cend());
      }
      catch (const nlohmann::json::parse_error& e)
      {
        this->RecordConfigurationError(name,
          QStringLiteral("resource '%1' is not valid JSON: %2").arg(value, QString::fromUtf8(e.what())));
      }
    }
    else
    {
      // A typo such as 'layoutLocks' would otherwise yield a silently unlocked
      // editor.
      this->RecordConfigurationError(name,
        QStringLiteral("unknown parameter (value '%1'); expected 'widgetName', 'layoutLock' or 'initialLayout'.")
          .arg(value));
    }
  }

  if (!QmitkMxNIsValidMultiWidgetName(m_Impl->m_WidgetName))
  {
    const auto rule = QStringLiteral("a widget name must start with a letter and contain only [A-Za-z0-9.-].");
    this->RecordConfigurationError(QStringLiteral("widgetName"), widgetNameDeclared
      ? QStringLiteral("'%1' is not valid; %2").arg(m_Impl->m_WidgetName, rule)
      : QStringLiteral("not declared, and the editor ID it defaults to is not valid; %1").arg(rule));
  }
}

void QmitkMxNMultiWidgetEditor::RecordConfigurationError(const QString& parameter, const QString& problem)
{
  const auto message = QStringLiteral("Editor '%1' declared by plugin '%2', parameter '%3': %4")
    .arg(m_Impl->m_EditorId, m_Impl->m_DeclaringPlugin, parameter, problem);
  MITK_ERROR << message.toStdString();
  m_Impl->m_ConfigurationErrors.append(message);
}

void QmitkMxNMultiWidgetEditor::ShowConfigurationErrors(QWidget* parent, const QString& reason)
{
  auto* label = new QLabel(parent);
  label->setTextFormat(Qt::PlainText);
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignCenter);
  label->setTextInteractionFlags(Qt::TextSelectableByMouse);
  label->setText(reason + QStringLiteral("\n\n") + m_Impl->m_ConfigurationErrors.join(QStringLiteral("\n")));
  parent->layout()->addWidget(label);
}

void QmitkMxNMultiWidgetEditor::PartOpened(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  // Identity, not the part ID: several declarations of this class have
  // different IDs, and an ID comparison would also fire for another instance
  // of the same declaration.
  if (partRef->GetPart(false).GetPointer() == this)
  {
    const auto& multiWidget = dynamic_cast<QmitkMxNMultiWidget*>(GetMultiWidget());
    if (nullptr != multiWidget)
    {
      multiWidget->EnableCrosshair();
    }
  }
}

void QmitkMxNMultiWidgetEditor::OnLayoutSet(int row, int column)
{
  const auto &multiWidget = dynamic_cast<QmitkMxNMultiWidget*>(GetMultiWidget());
  if (nullptr != multiWidget)
  {
    QmitkAbstractMultiWidgetEditor::OnLayoutSet(row, column);
    multiWidget->EnableCrosshair();
  }
}

void QmitkMxNMultiWidgetEditor::OnInteractionSchemeApplied(mitk::InteractionSchemeSwitcher::InteractionScheme scheme)
{
  m_Impl->m_InteractionScheme = scheme;
  if (nullptr == m_Impl->m_InteractionSchemeToolBar)
  {
    return;
  }
  m_Impl->m_InteractionSchemeToolBar->setVisible(QmitkAbstractMultiWidget::IsPACSScheme(scheme));
  m_Impl->m_InteractionSchemeToolBar->SetInteractionScheme(scheme);
}

mitk::InteractionSchemeSwitcher::InteractionScheme
QmitkMxNMultiWidgetEditor::GetInteractionScheme() const
{
  return m_Impl->m_InteractionScheme;
}

bool QmitkMxNMultiWidgetEditor::HasConfigurationErrors() const
{
  return !m_Impl->m_ConfigurationErrors.isEmpty();
}

//////////////////////////////////////////////////////////////////////////
// PRIVATE
//////////////////////////////////////////////////////////////////////////
void QmitkMxNMultiWidgetEditor::SetFocus()
{
  const auto& multiWidget = GetMultiWidget();
  if (nullptr != multiWidget)
  {
    multiWidget->setFocus();
  }
}

void QmitkMxNMultiWidgetEditor::CreateQtPartControl(QWidget* parent)
{
  QHBoxLayout *layout = new QHBoxLayout(parent);
  layout->setContentsMargins(0, 0, 0, 0);

  const auto invalidDeclaration = QStringLiteral("This display cannot be shown because its declaration is invalid.");
  if (!m_Impl->m_ConfigurationErrors.isEmpty())
  {
    this->ShowConfigurationErrors(parent, invalidDeclaration);
    return;
  }

  auto* preferences = this->GetPreferences();

  // Locks are set before InitializeMultiWidget creates the first cell, so no
  // cell is ever shown unlocked. The initial layout is applied before the
  // widget is registered anywhere, so a failing one can be undone by deleting
  // the widget: nothing outside this method has seen it. The constructor
  // rejects a name another live widget already uses.
  QmitkMxNMultiWidget* multiWidget = nullptr;
  try
  {
    multiWidget = new QmitkMxNMultiWidget(parent, {}, m_Impl->m_WidgetName);
  }
  catch (const QmitkMxNWidgetNameInUseException& e)
  {
    this->RecordConfigurationError(QStringLiteral("widgetName"), QString::fromStdString(e.GetDescription()));
    this->ShowConfigurationErrors(parent,
      QStringLiteral("This display cannot be shown because another open MxN display already uses its widget name."));
    return;
  }
  multiWidget->SetDataStorage(this->GetDataStorage());
  multiWidget->SetLayoutLocks(m_Impl->m_LayoutLocks);
  multiWidget->InitializeMultiWidget();
  if (m_Impl->m_InitialLayout.has_value())
  {
    try
    {
      multiWidget->ApplyLayout(*m_Impl->m_InitialLayout);
    }
    catch (const mitk::Exception& e)
    {
      delete multiWidget;
      this->RecordConfigurationError(QStringLiteral("initialLayout"), QString::fromStdString(e.GetDescription()));
      this->ShowConfigurationErrors(parent, invalidDeclaration);
      return;
    }
    m_Impl->m_InitialLayout.reset();
  }

  // create left toolbar: interaction scheme toolbar to switch how the render window navigation behaves in PACS mode
  m_Impl->m_InteractionSchemeToolBar = new QmitkInteractionSchemeToolBar(parent);
  // Keeps the tool bar from showing before the applied scheme is known; the
  // sync at the end of this method decides whether it stays hidden.
  m_Impl->m_InteractionSchemeToolBar->setVisible(false);
  layout->addWidget(m_Impl->m_InteractionSchemeToolBar);

  // The tool bar only requests a scheme; the multi widget reports back what it
  // applied. Keeping the two directions apart is what lets the tool bars follow
  // scheme changes that did not originate from them.
  connect(m_Impl->m_InteractionSchemeToolBar, &QmitkInteractionSchemeToolBar::InteractionSchemeChanged,
    this, &QmitkMxNMultiWidgetEditor::OnInteractionSchemeChanged);

  this->SetMultiWidget(multiWidget);
  connect(multiWidget, &QmitkMxNMultiWidget::LayoutChanged,
    this, &QmitkMxNMultiWidgetEditor::OnLayoutChanged);
  connect(multiWidget, &QmitkMxNMultiWidget::LayoutEditorRequested,
    this, &QmitkMxNMultiWidgetEditor::OnLayoutEditorRequested);
  connect(multiWidget, &QmitkAbstractMultiWidget::InteractionSchemeChanged,
    this, &QmitkMxNMultiWidgetEditor::OnInteractionSchemeApplied);

  layout->addWidget(multiWidget);

  // No configuration toolbar: the editor-wide controls, the interaction-scheme
  // switch included, live in the MxN Layout Editor view.

  GetSite()->GetPage()->AddPartListener(this);

  this->OnPreferencesChanged(preferences);

  this->OnInteractionSchemeApplied(multiWidget->GetInteractionScheme());
}

void QmitkMxNMultiWidgetEditor::OnPreferencesChanged(const mitk::IPreferences* preferences)
{
  const auto& multiWidget = GetMultiWidget();
  if (nullptr == multiWidget)
  {
    return;
  }

  int crosshairGapSize = preferences->GetInt("crosshair gap size", 32);
  multiWidget->SetCrosshairGap(crosshairGapSize);

  if (auto* mxnMultiWidget = dynamic_cast<QmitkMxNMultiWidget*>(multiWidget))
  {
    mxnMultiWidget->SetLevelWindowReadoutVisible(
      preferences->GetBool("Show level/window readout", true));
    mxnMultiWidget->SetNavigatorExpanded(preferences->GetBool("Expanded navigator", false));
  }

  // Only a change of the preference itself overrides the interaction scheme, so
  // that the PACS tool the user picked survives unrelated preference edits. A
  // change that the active scheme already agrees with is no reason to reset it
  // either.
  const bool pacsInteraction = preferences->GetBool("PACS like mouse interaction", false);
  const bool preferenceChanged = m_Impl->m_PACSInteraction != pacsInteraction;
  m_Impl->m_PACSInteraction = pacsInteraction;

  if (preferenceChanged && pacsInteraction != QmitkAbstractMultiWidget::IsPACSScheme(multiWidget->GetInteractionScheme()))
  {
    this->OnInteractionSchemeChanged(pacsInteraction ?
      mitk::InteractionSchemeSwitcher::PACSStandard :
      mitk::InteractionSchemeSwitcher::MITKStandard);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMxNMultiWidgetEditor::OnLayoutChanged()
{
  FirePropertyChange(berry::IWorkbenchPartConstants::PROP_INPUT);
}

void QmitkMxNMultiWidgetEditor::OnLayoutEditorRequested(QmitkMxNMultiWidget::LayoutEditorRequest request)
{
  // A toggle request hides an already-visible layout editor instead of
  // re-activating it, so a second press on the barcode closes what the first
  // opened; a show request only ever brings the editor up, a hide request only
  // ever takes it down.
  auto page = this->GetSite()->GetPage();
  if (page.IsNull())
  {
    return;
  }

  const QString viewId = QStringLiteral("org.mitk.views.mxnlayouteditor");
  auto view = page->FindView(viewId);
  if (QmitkMxNMultiWidget::LayoutEditorRequest::Hide == request)
  {
    if (view.IsNotNull())
    {
      page->HideView(view);
    }
    return;
  }
  if (QmitkMxNMultiWidget::LayoutEditorRequest::Toggle == request && view.IsNotNull()
      && page->IsPartVisible(view))
  {
    page->HideView(view);
    return;
  }

  try
  {
    page->ShowView(viewId);
  }
  catch (const berry::PartInitException& e)
  {
    MITK_ERROR << "Could not open the MxN layout editor view: " << e.what();
  }
}
