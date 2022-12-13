/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkStdMultiWidgetEditor.h"

#include <berryUIException.h>
#include <berryIWorkbenchPage.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <mitkColorProperty.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

#include <mitkDataStorageEditorInput.h>
#include <mitkIDataStorageService.h>

// mitk qt widgets module
#include <QmitkInteractionSchemeToolBar.h>
#include <QmitkLevelWindowWidget.h>
#include <QmitkRenderWindowWidget.h>
#include <QmitkStdMultiWidget.h>

// mitk gui qt common plugin
#include <QmitkMultiWidgetDecorationManager.h>

const QString QmitkStdMultiWidgetEditor::EDITOR_ID = "org.mitk.editors.stdmultiwidget";

struct QmitkStdMultiWidgetEditor::Impl final
{
  Impl();
  ~Impl() = default;

  QmitkInteractionSchemeToolBar* m_InteractionSchemeToolBar;
  QmitkLevelWindowWidget* m_LevelWindowWidget;

  std::unique_ptr<QmitkMultiWidgetDecorationManager> m_MultiWidgetDecorationManager;
};

QmitkStdMultiWidgetEditor::Impl::Impl()
  : m_InteractionSchemeToolBar(nullptr)
  , m_LevelWindowWidget(nullptr)
{
  // nothing here
}

//////////////////////////////////////////////////////////////////////////
// QmitkStdMultiWidgetEditor
//////////////////////////////////////////////////////////////////////////
QmitkStdMultiWidgetEditor::QmitkStdMultiWidgetEditor()
  : QmitkAbstractMultiWidgetEditor()
  , m_Impl(std::make_unique<Impl>())
{
  // nothing here
}

QmitkStdMultiWidgetEditor::~QmitkStdMultiWidgetEditor()
{
  GetSite()->GetPage()->RemovePartListener(this);
}

berry::IPartListener::Events::Types QmitkStdMultiWidgetEditor::GetPartEventTypes() const
{
  return Events::CLOSED | Events::OPENED | Events::HIDDEN | Events::VISIBLE;
}

void QmitkStdMultiWidgetEditor::PartClosed(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  if (partRef->GetId() == QmitkStdMultiWidgetEditor::EDITOR_ID)
  {
    const auto& multiWidget = dynamic_cast<QmitkStdMultiWidget*>(GetMultiWidget());
    if (nullptr != multiWidget)
    {
      multiWidget->RemovePlanesFromDataStorage();
      multiWidget->ActivateMenuWidget(false);
    }
  }
}

void QmitkStdMultiWidgetEditor::PartOpened(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  if (partRef->GetId() == QmitkStdMultiWidgetEditor::EDITOR_ID)
  {
    const auto& multiWidget = dynamic_cast<QmitkStdMultiWidget*>(GetMultiWidget());
    if (nullptr != multiWidget)
    {
      multiWidget->AddPlanesToDataStorage();
      multiWidget->ActivateMenuWidget(true);
    }
  }
}

void QmitkStdMultiWidgetEditor::PartHidden(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  if (partRef->GetId() == QmitkStdMultiWidgetEditor::EDITOR_ID)
  {
    const auto& multiWidget = dynamic_cast<QmitkStdMultiWidget*>(GetMultiWidget());
    if (nullptr != multiWidget)
    {
      multiWidget->ActivateMenuWidget(false);
    }
  }
}

void QmitkStdMultiWidgetEditor::PartVisible(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  if (partRef->GetId() == QmitkStdMultiWidgetEditor::EDITOR_ID)
  {
    const auto& multiWidget = dynamic_cast<QmitkStdMultiWidget*>(GetMultiWidget());
    if (nullptr != multiWidget)
    {
      multiWidget->ActivateMenuWidget(true);
    }
  }
}

QmitkLevelWindowWidget* QmitkStdMultiWidgetEditor::GetLevelWindowWidget() const
{
  return m_Impl->m_LevelWindowWidget;
}

void QmitkStdMultiWidgetEditor::EnableSlicingPlanes(bool enable)
{
  const auto& multiWidget = dynamic_cast<QmitkStdMultiWidget*>(GetMultiWidget());
  if (nullptr == multiWidget)
  {
    return;
  }

  multiWidget->SetWidgetPlanesVisibility(enable);
}

bool QmitkStdMultiWidgetEditor::IsSlicingPlanesEnabled() const
{
  const auto& multiWidget = dynamic_cast<QmitkStdMultiWidget*>(GetMultiWidget());
  if (nullptr == multiWidget)
  {
    return false;
  }

  mitk::DataNode::Pointer node = multiWidget->GetWidgetPlane1();
  if (node.IsNotNull())
  {
    bool visible = false;
    node->GetVisibility(visible, nullptr);
    return visible;
  }
  else
  {
    return false;
  }
}

void QmitkStdMultiWidgetEditor::OnInteractionSchemeChanged(mitk::InteractionSchemeSwitcher::InteractionScheme scheme)
{
  const auto& multiWidget = GetMultiWidget();
  if (nullptr == multiWidget)
  {
    return;
  }

  if (mitk::InteractionSchemeSwitcher::PACSStandard == scheme)
  {
    m_Impl->m_InteractionSchemeToolBar->setVisible(true);
  }
  else
  {
    m_Impl->m_InteractionSchemeToolBar->setVisible(false);
  }

  QmitkAbstractMultiWidgetEditor::OnInteractionSchemeChanged(scheme);
}

void QmitkStdMultiWidgetEditor::ShowLevelWindowWidget(bool show)
{
  if (show)
  {
    m_Impl->m_LevelWindowWidget->disconnect(this);
    m_Impl->m_LevelWindowWidget->SetDataStorage(GetDataStorage());
    m_Impl->m_LevelWindowWidget->show();
  }
  else
  {
    m_Impl->m_LevelWindowWidget->disconnect(this);
    m_Impl->m_LevelWindowWidget->hide();
  }
}

void QmitkStdMultiWidgetEditor::SetFocus()
{
  const auto& multiWidget = GetMultiWidget();
  if (nullptr != multiWidget)
  {
    multiWidget->setFocus();
  }
}

void QmitkStdMultiWidgetEditor::CreateQtPartControl(QWidget* parent)
{
  QHBoxLayout* layout = new QHBoxLayout(parent);
  layout->setContentsMargins(0, 0, 0, 0);

  auto* preferences = this->GetPreferences();

  auto multiWidget = GetMultiWidget();
  if (nullptr == multiWidget)
  {
    multiWidget = new QmitkStdMultiWidget(parent);

    // create left toolbar: interaction scheme toolbar to switch how the render window navigation behaves (in PACS mode)
    if (nullptr == m_Impl->m_InteractionSchemeToolBar)
    {
      m_Impl->m_InteractionSchemeToolBar = new QmitkInteractionSchemeToolBar(parent);
      layout->addWidget(m_Impl->m_InteractionSchemeToolBar);
    }
    m_Impl->m_InteractionSchemeToolBar->SetInteractionEventHandler(multiWidget->GetInteractionEventHandler());

    multiWidget->SetDataStorage(GetDataStorage());
    multiWidget->InitializeMultiWidget();
    SetMultiWidget(multiWidget);
  }

  layout->addWidget(multiWidget);

  // create level window slider on the right side
  if (nullptr == m_Impl->m_LevelWindowWidget)
  {
    m_Impl->m_LevelWindowWidget = new QmitkLevelWindowWidget(parent);
    m_Impl->m_LevelWindowWidget->setObjectName(QString::fromUtf8("levelWindowWidget"));

    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(m_Impl->m_LevelWindowWidget->sizePolicy().hasHeightForWidth());
    m_Impl->m_LevelWindowWidget->setSizePolicy(sizePolicy);
    m_Impl->m_LevelWindowWidget->setMaximumWidth(50);
  }

  layout->addWidget(m_Impl->m_LevelWindowWidget);

  m_Impl->m_MultiWidgetDecorationManager = std::make_unique<QmitkMultiWidgetDecorationManager>(multiWidget);

  GetSite()->GetPage()->AddPartListener(this);

  InitializePreferences(preferences);
  OnPreferencesChanged(preferences);
}

void QmitkStdMultiWidgetEditor::OnPreferencesChanged(const mitk::IPreferences* preferences)
{
  const auto& multiWidget = dynamic_cast<QmitkStdMultiWidget*>(GetMultiWidget());
  if (nullptr == multiWidget)
  {
    return;
  }

  // change and apply decoration preferences
  GetPreferenceDecorations(preferences);
  m_Impl->m_MultiWidgetDecorationManager->DecorationPreferencesChanged(preferences);

  QmitkAbstractMultiWidget::RenderWindowWidgetMap renderWindowWidgets = multiWidget->GetRenderWindowWidgets();
  int i = 0;
  for (const auto& renderWindowWidget : renderWindowWidgets)
  {
    auto decorationColor = renderWindowWidget.second->GetDecorationColor();
    multiWidget->SetDecorationColor(i, decorationColor);

    ++i;
  }

  int crosshairGapSize = preferences->GetInt("crosshair gap size", 32);
  multiWidget->SetCrosshairGap(crosshairGapSize);

  // zooming and panning preferences
  bool constrainedZooming = preferences->GetBool("Use constrained zooming and panning", true);
  mitk::RenderingManager::GetInstance()->SetConstrainedPanningZooming(constrainedZooming);

  // mouse modes switcher toolbar
  bool PACSInteractionScheme = preferences->GetBool("PACS like mouse interaction", false);
  OnInteractionSchemeChanged(PACSInteractionScheme ?
    mitk::InteractionSchemeSwitcher::PACSStandard :
    mitk::InteractionSchemeSwitcher::MITKStandard);

  // level window setting
  bool showLevelWindowWidget = preferences->GetBool("Show level/window widget", true);
  ShowLevelWindowWidget(showLevelWindowWidget);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkStdMultiWidgetEditor::InitializePreferences(mitk::IPreferences * preferences)
{
  auto multiWidget = this->GetMultiWidget();

  if (nullptr == multiWidget)
    return;

  this->GetPreferenceDecorations(preferences); // Override if preferences are defined

  for (const auto& renderWindowWidget : multiWidget->GetRenderWindowWidgets())
  {
    auto widgetName = renderWindowWidget.second->GetWidgetName().toStdString();

    auto gradientBackgroundColors = renderWindowWidget.second->GetGradientBackgroundColors();
    preferences->Put(widgetName + " first background color", this->MitkColorToHex(gradientBackgroundColors.first));
    preferences->Put(widgetName + " second background color", this->MitkColorToHex(gradientBackgroundColors.second));

    auto decorationColor = renderWindowWidget.second->GetDecorationColor();
    preferences->Put(widgetName + " decoration color", this->MitkColorToHex(decorationColor));

    auto cornerAnnotation = renderWindowWidget.second->GetCornerAnnotationText();
    preferences->Put(widgetName + " corner annotation", cornerAnnotation);
  }
}

void QmitkStdMultiWidgetEditor::GetPreferenceDecorations(const mitk::IPreferences * preferences)
{
  auto multiWidget = dynamic_cast<QmitkStdMultiWidget*>(GetMultiWidget());

  if (nullptr == multiWidget)
    return;

  auto hexBlack = "#000000";
  auto gradientBlack = "#191919";
  auto gradientGray = "#7F7F7F";

  auto renderWindowWidgets = multiWidget->GetRenderWindowWidgets();
  int i = 0;
  for (const auto& renderWindowWidget : renderWindowWidgets)
  {
    auto widgetName = renderWindowWidget.second->GetWidgetName().toStdString();

    if (mitk::BaseRenderer::Standard3D == mitk::BaseRenderer::GetInstance(renderWindowWidget.second->GetRenderWindow()->GetVtkRenderWindow())->GetMapperID())
    {
      auto upper = preferences->Get(widgetName + " first background color", gradientBlack);
      auto lower = preferences->Get(widgetName + " second background color", gradientGray);
      renderWindowWidget.second->SetGradientBackgroundColors(HexColorToMitkColor(upper), HexColorToMitkColor(lower));
    }
    else
    {
      auto upper = preferences->Get(widgetName + " first background color", hexBlack);
      auto lower = preferences->Get(widgetName + " second background color", hexBlack);
      renderWindowWidget.second->SetGradientBackgroundColors(HexColorToMitkColor(upper), HexColorToMitkColor(lower));
    }

    auto defaultDecorationColor = multiWidget->GetDecorationColor(i);
    auto decorationColor = preferences->Get(widgetName + " decoration color", MitkColorToHex(defaultDecorationColor));
    renderWindowWidget.second->SetDecorationColor(HexColorToMitkColor(decorationColor));

    auto defaultCornerAnnotation = renderWindowWidget.second->GetCornerAnnotationText();
    auto cornerAnnotation = preferences->Get(widgetName + " corner annotation", defaultCornerAnnotation);
    renderWindowWidget.second->SetCornerAnnotationText(cornerAnnotation);

    ++i;
  }
}

mitk::Color QmitkStdMultiWidgetEditor::HexColorToMitkColor(const std::string& hexColor)
{
  QColor qColor(hexColor.c_str());
  mitk::Color returnColor;
  float colorMax = 255.0f;
  if (hexColor.empty()) // default value
  {
    returnColor[0] = 1.0;
    returnColor[1] = 1.0;
    returnColor[2] = 1.0;
    MITK_ERROR << "Using default color for unknown hex color " << hexColor;
  }
  else
  {
    returnColor[0] = qColor.red() / colorMax;
    returnColor[1] = qColor.green() / colorMax;
    returnColor[2] = qColor.blue() / colorMax;
  }
  return returnColor;
}

std::string QmitkStdMultiWidgetEditor::MitkColorToHex(const mitk::Color& color)
{
  QColor returnColor;
  float colorMax = 255.0f;
  returnColor.setRed(static_cast<int>(color[0] * colorMax + 0.5));
  returnColor.setGreen(static_cast<int>(color[1] * colorMax + 0.5));
  returnColor.setBlue(static_cast<int>(color[2] * colorMax + 0.5));
  return returnColor.name().toStdString();
}
