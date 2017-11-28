/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkCustomMultiWidgetEditor.h"

#include <berryUIException.h>
#include <berryIWorkbenchPage.h>
#include <berryIPreferencesService.h>
#include <berryIPreferences.h>

#include <QWidget>

#include <mitkColorProperty.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

#include <mitkDataStorageEditorInput.h>
#include <mitkIDataStorageService.h>

#include <QmitkMouseModeSwitcher.h>
#include <QmitkCustomMultiWidget.h>


//#TODO: create different default views (different layouts of the render window widgets)
class QmitkCustomMultiWidgetEditor::Impl final
{
public:

  Impl();
  ~Impl();

  QmitkCustomMultiWidget* m_CustomMultiWidget;
  QmitkMouseModeSwitcher* m_MouseModeToolbar;
  /*
  * @brief Members for the MultiWidget decoration
  */
  QString m_WidgetBackgroundColor1[4];
  QString m_WidgetBackgroundColor2[4];
  QString m_WidgetDecorationColor[4];
  QString m_WidgetAnnotation[4];
  bool m_MenuWidgetsEnabled;

  QHash<QString, QmitkRenderWindow*> m_RenderWindows;

};

QmitkCustomMultiWidgetEditor::Impl::Impl()
  : m_CustomMultiWidget(nullptr)
  , m_MouseModeToolbar(nullptr)
  , m_MenuWidgetsEnabled(false)
{
  // nothing here
}

QmitkCustomMultiWidgetEditor::Impl::~Impl()
{
  // nothing here
}

const QString QmitkCustomMultiWidgetEditor::EDITOR_ID = "org.mitk.editors.custommultiwidget";

QmitkCustomMultiWidgetEditor::QmitkCustomMultiWidgetEditor()
  : m_Impl(new Impl()) // this?
{
  m_Impl->m_CustomMultiWidget->AddPlanesToDataStorage();
  RequestActivateMenuWidget(true);
}

QmitkCustomMultiWidgetEditor::~QmitkCustomMultiWidgetEditor()
{
  m_Impl->m_CustomMultiWidget->RemovePlanesFromDataStorage();
  RequestActivateMenuWidget(false);
}

void QmitkCustomMultiWidgetEditor::Activated()
{
  // nothing here
}

void QmitkCustomMultiWidgetEditor::Deactivated()
{
  // nothing here
}

void QmitkCustomMultiWidgetEditor::Visible()
{
  RequestActivateMenuWidget(true);
}

void QmitkCustomMultiWidgetEditor::Hidden()
{
  RequestActivateMenuWidget(false);
}

QmitkRenderWindow* QmitkCustomMultiWidgetEditor::GetActiveQmitkRenderWindow() const
{
  if (m_Impl->m_CustomMultiWidget)
  {
    return m_Impl->m_CustomMultiWidget->GetActiveRenderWindowWidget();
  }

  return nullptr;
}

QHash<QString, QmitkRenderWindow*> QmitkCustomMultiWidgetEditor::GetQmitkRenderWindows() const
{
  return m_Impl->m_RenderWindows;
}

QmitkRenderWindow* QmitkCustomMultiWidgetEditor::GetQmitkRenderWindow(const QString &id) const
{
  if (m_Impl->m_RenderWindows.contains(id))
  {
    return m_Impl->m_RenderWindows[id];
  }

  return nullptr;
}

mitk::Point3D QmitkCustomMultiWidgetEditor::GetSelectedPosition(const QString & /*id*/) const
{
  return m_Impl->m_CustomMultiWidget->GetCrossPosition();
}

void QmitkCustomMultiWidgetEditor::SetSelectedPosition(const mitk::Point3D &pos, const QString &/*id*/)
{
  m_Impl->m_CustomMultiWidget->MoveCrossToPosition(pos);
}

void QmitkCustomMultiWidgetEditor::EnableDecorations(bool enable, const QStringList &decorations)
{
  if (decorations.isEmpty() || decorations.contains(DECORATION_BORDER))
  {
    enable ? m_Impl->m_CustomMultiWidget->EnableColoredRectangles()
           : m_Impl->m_CustomMultiWidget->DisableColoredRectangles();
  }
  if (decorations.isEmpty() || decorations.contains(DECORATION_LOGO))
  {
    enable ? m_Impl->m_CustomMultiWidget->EnableDepartmentLogo()
           : m_Impl->m_CustomMultiWidget->DisableDepartmentLogo();
  }
  if (decorations.isEmpty() || decorations.contains(DECORATION_MENU))
  {
    m_Impl->m_CustomMultiWidget->ActivateRenderWindowWidgetMenu(enable);
  }
  if (decorations.isEmpty() || decorations.contains(DECORATION_BACKGROUND))
  {
    enable ? m_Impl->m_CustomMultiWidget->EnableGradientBackground()
           : m_Impl->m_CustomMultiWidget->DisableGradientBackground();
  }
  if (decorations.isEmpty() || decorations.contains(DECORATION_CORNER_ANNOTATION))
  {
    enable ? m_Impl->m_CustomMultiWidget->SetCornerAnnotationVisibility(true)
           : m_Impl->m_CustomMultiWidget->SetCornerAnnotationVisibility(false);
  }
}

bool QmitkCustomMultiWidgetEditor::IsDecorationEnabled(const QString &decoration) const
{
  if (decoration == DECORATION_BORDER)
  {
    return m_Impl->m_CustomMultiWidget->IsColoredRectanglesEnabled();
  }
  else if (decoration == DECORATION_LOGO)
  {
    return m_Impl->m_CustomMultiWidget->IsColoredRectanglesEnabled();
  }
  else if (decoration == DECORATION_MENU)
  {
    return m_Impl->m_CustomMultiWidget->IsRenderWindowWidgetMenuEnabled();
  }
  else if (decoration == DECORATION_BACKGROUND)
  {
    return m_Impl->m_CustomMultiWidget->GetGradientBackgroundFlag();
  }
  else if (decoration == DECORATION_CORNER_ANNOTATION)
  {
    return m_Impl->m_CustomMultiWidget->IsCornerAnnotationVisible();
  }

  return false;
}

QStringList QmitkCustomMultiWidgetEditor::GetDecorations() const
{
  QStringList decorations;
  decorations << DECORATION_BORDER << DECORATION_LOGO << DECORATION_MENU << DECORATION_BACKGROUND << DECORATION_CORNER_ANNOTATION;
  return decorations;
}

void QmitkCustomMultiWidgetEditor::EnableSlicingPlanes(bool enable)
{
  m_Impl->m_CustomMultiWidget->SetWidgetPlanesVisibility(enable);
}

bool QmitkCustomMultiWidgetEditor::IsSlicingPlanesEnabled() const
{
  mitk::DataNode::Pointer node = m_Impl->m_CustomMultiWidget->GetWidgetPlane1();
  if (node.IsNotNull())
  {
    bool visible = false;
    node->GetVisibility(visible, 0);
    return visible;
  }
  else
  {
    return false;
  }
}

QmitkCustomMultiWidget* QmitkCustomMultiWidgetEditor::GetCustomMultiWidget()
{
  return m_Impl->m_CustomMultiWidget;
}

void QmitkCustomMultiWidgetEditor::SetFocus()
{
  if (nullptr != m_Impl->m_CustomMultiWidget)
  {
    m_Impl->m_CustomMultiWidget->setFocus();
  }
}

void QmitkCustomMultiWidgetEditor::OnPreferencesChanged(const berry::IBerryPreferences* preferences)
{
  // Enable change of logo. If no DepartmentLogo was set explicitly, MBILogo is used.
  // Set new department logo by prefs->Set("DepartmentLogo", "PathToImage");

  // If no logo was set for this plug-in specifically, walk the parent preference nodes
  // and lookup a logo value there.

  const berry::IPreferences* currentNode = preferences;

  while(currentNode)
  {
    bool logoFound = false;
    foreach (const QString& key, currentNode->Keys())
    {
      if( key == "DepartmentLogo")
      {
        QString departmentLogoLocation = currentNode->Get("DepartmentLogo", "");

        if (departmentLogoLocation.isEmpty())
        {
          m_Impl->m_CustomMultiWidget->DisableDepartmentLogo();
        }
        else
        {
          // we need to disable the logo first, otherwise setting a new logo will have
          // no effect due to how mitkManufacturerLogo works...
          m_Impl->m_CustomMultiWidget->DisableDepartmentLogo();
          m_Impl->m_CustomMultiWidget->SetDepartmentLogoPath(qPrintable(departmentLogoLocation));
          m_Impl->m_CustomMultiWidget->EnableDepartmentLogo();
        }
        logoFound = true;
        break;
      }
    }

    if (logoFound) break;
    currentNode = currentNode->Parent().GetPointer();
  }

  // Update internal members
  FillMembersWithCurrentDecorations();
  GetPreferenceDecorations(preferences);
  // Now the members can be used to modify the custom multi widget
  mitk::Color upper = HexColorToMitkColor(m_Impl->m_WidgetBackgroundColor1[0]);
  mitk::Color lower = HexColorToMitkColor(m_Impl->m_WidgetBackgroundColor2[0]);
  m_Impl->m_CustomMultiWidget->SetBackgroundColorGradient(upper, lower, 0);
  m_Impl->m_CustomMultiWidget->ShowBackgroundColorGradient(0, true);
  upper = HexColorToMitkColor(m_Impl->m_WidgetBackgroundColor1[1]);
  lower = HexColorToMitkColor(m_Impl->m_WidgetBackgroundColor2[1]);
  m_Impl->m_CustomMultiWidget->SetBackgroundColorGradient(upper, lower, 1);
  m_Impl->m_CustomMultiWidget->ShowBackgroundColorGradient(1, true);
  upper = HexColorToMitkColor(m_Impl->m_WidgetBackgroundColor1[2]);
  lower = HexColorToMitkColor(m_Impl->m_WidgetBackgroundColor2[2]);
  m_Impl->m_CustomMultiWidget->SetBackgroundColorGradient(upper, lower, 2);
  m_Impl->m_CustomMultiWidget->ShowBackgroundColorGradient(2, true);
  upper = HexColorToMitkColor(m_Impl->m_WidgetBackgroundColor1[3]);
  lower = HexColorToMitkColor(m_Impl->m_WidgetBackgroundColor2[3]);
  m_Impl->m_CustomMultiWidget->SetBackgroundColorGradient(upper, lower, 3);
  m_Impl->m_CustomMultiWidget->ShowBackgroundColorGradient(3, true);

  // preferences for renderWindows
  mitk::Color colorDecorationWidget1 = HexColorToMitkColor(m_Impl->m_WidgetDecorationColor[0]);
  mitk::Color colorDecorationWidget2 = HexColorToMitkColor(m_Impl->m_WidgetDecorationColor[1]);
  mitk::Color colorDecorationWidget3 = HexColorToMitkColor(m_Impl->m_WidgetDecorationColor[2]);
  mitk::Color colorDecorationWidget4 = HexColorToMitkColor(m_Impl->m_WidgetDecorationColor[3]);
  m_Impl->m_CustomMultiWidget->SetDecorationColor(0, colorDecorationWidget1);
  m_Impl->m_CustomMultiWidget->SetDecorationColor(1, colorDecorationWidget2);
  m_Impl->m_CustomMultiWidget->SetDecorationColor(2, colorDecorationWidget3);
  m_Impl->m_CustomMultiWidget->SetDecorationColor(3, colorDecorationWidget4);

  for(unsigned int i = 0; i < 4; ++i)
  {
    m_Impl->m_CustomMultiWidget->SetDecorationProperties(m_Impl->m_WidgetAnnotation[i].toStdString(), HexColorToMitkColor(m_Impl->m_WidgetDecorationColor[i]), i);
  }
  // The crosshair gap
  int crosshairgapsize = preferences->GetInt("crosshair gap size", 32);
  m_Impl->m_CustomMultiWidget->GetWidgetPlane1()->SetIntProperty("Crosshair.Gap Size", crosshairgapsize);
  m_Impl->m_CustomMultiWidget->GetWidgetPlane2()->SetIntProperty("Crosshair.Gap Size", crosshairgapsize);
  m_Impl->m_CustomMultiWidget->GetWidgetPlane3()->SetIntProperty("Crosshair.Gap Size", crosshairgapsize);

  //refresh colors of rectangles
  m_Impl->m_CustomMultiWidget->EnableColoredRectangles();

  // Set preferences respecting zooming and panning
  bool constrainedZooming = preferences->GetBool("Use constrained zooming and panning", true);

  mitk::RenderingManager::GetInstance()->SetConstrainedPanningZooming(constrainedZooming);

  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  // level window setting
  bool showLevelWindowWidget = preferences->GetBool("Show level/window widget", true);
  if (showLevelWindowWidget)
  {
    m_Impl->m_CustomMultiWidget->ShowAllLevelWindowWidgets(true);
  }
  else
  {
    m_Impl->m_CustomMultiWidget->ShowAllLevelWindowWidgets(false);
  }

  // mouse modes toolbar
  bool newMode = preferences->GetBool("PACS like mouse interaction", false);
  m_Impl->m_MouseModeToolbar->setVisible( newMode );
  m_Impl->m_CustomMultiWidget->GetMouseModeSwitcher()->SetInteractionScheme( newMode ? mitk::MouseModeSwitcher::PACS : mitk::MouseModeSwitcher::MITK );
}

void QmitkCustomMultiWidgetEditor::CreateQtPartControl(QWidget* parent)
{
  if (nullptr == m_Impl->m_CustomMultiWidget)
  {
    QHBoxLayout* layout = new QHBoxLayout(parent);
    layout->setContentsMargins(0, 0, 0, 0);

    if (nullptr == m_Impl->m_MouseModeToolbar)
    {
      m_Impl->m_MouseModeToolbar = new QmitkMouseModeSwitcher(parent);
      layout->addWidget(m_Impl->m_MouseModeToolbar);
    }

    berry::IPreferences::Pointer prefs = GetPreferences();
    mitk::BaseRenderer::RenderingMode::Type renderingMode = static_cast<mitk::BaseRenderer::RenderingMode::Type>(prefs->GetInt("Rendering Mode", 0));

    m_Impl->m_CustomMultiWidget = new QmitkCustomMultiWidget(parent, 0, 0, renderingMode);
    m_Impl->m_RenderWindows.insert("axial", m_Impl->m_CustomMultiWidget->GetRenderWindowWidget());
    layout->addWidget(m_Impl->m_CustomMultiWidget);

    m_Impl->m_MouseModeToolbar->setMouseModeSwitcher(m_Impl->m_CustomMultiWidget->GetMouseModeSwitcher());

    mitk::DataStorage::Pointer dataStorage = GetDataStorage();
    m_Impl->m_CustomMultiWidget->SetDataStorage(dataStorage);

    mitk::TimeGeometry::Pointer timeGeometry = dataStorage->ComputeBoundingGeometry3D(dataStorage->GetAll());
    mitk::RenderingManager::GetInstance()->InitializeViews(timeGeometry);

    m_Impl->m_CustomMultiWidget->ShowAllLevelWindowWidgets(true);

    // Add the displayed views to the tree to see their positions
    // in 2D and 3D
    m_Impl->m_CustomMultiWidget->AddDisplayPlaneSubTree();

    //m_Impl->m_StdMultiWidget->EnableNavigationControllerEventListening();

    // Store the initial visibility status of the menu widget.
    m_Impl->m_MenuWidgetsEnabled = m_Impl->m_CustomMultiWidget->IsRenderWindowWidgetMenuEnabled();

    berry::IBerryPreferences* berryprefs = dynamic_cast<berry::IBerryPreferences*>(prefs.GetPointer());
    InitializePreferences(berryprefs);
    OnPreferencesChanged(berryprefs);

    RequestUpdate();
  }
}

void QmitkCustomMultiWidgetEditor::RequestActivateMenuWidget(bool on)
{
  if (nullptr == m_Impl->m_CustomMultiWidget)
  {
    return;
  }

  if (on)
  {
    m_Impl->m_CustomMultiWidget->ActivateAllRenderWindowMenus(m_Impl->m_MenuWidgetsEnabled);
  }
  else
  {
    m_Impl->m_MenuWidgetsEnabled = m_Impl->m_CustomMultiWidget->IsRenderWindowWidgetMenuEnabled();
    m_Impl->m_CustomMultiWidget->ActivateAllRenderWindowMenus(false);
  }
}

void QmitkCustomMultiWidgetEditor::InitializePreferences(berry::IBerryPreferences* preferences)
{
  FillMembersWithCurrentDecorations(); // fill members
  GetPreferenceDecorations(preferences); // overwrite if preferences are defined

                                         //create new preferences
  preferences->Put("widget1 corner annotation", m_Impl->m_WidgetAnnotation[0]);
  preferences->Put("widget2 corner annotation", m_Impl->m_WidgetAnnotation[1]);
  preferences->Put("widget3 corner annotation", m_Impl->m_WidgetAnnotation[2]);
  preferences->Put("widget4 corner annotation", m_Impl->m_WidgetAnnotation[3]);

  preferences->Put("widget1 decoration color", m_Impl->m_WidgetDecorationColor[0]);
  preferences->Put("widget2 decoration color", m_Impl->m_WidgetDecorationColor[1]);
  preferences->Put("widget3 decoration color", m_Impl->m_WidgetDecorationColor[2]);
  preferences->Put("widget4 decoration color", m_Impl->m_WidgetDecorationColor[3]);

  preferences->Put("widget1 first background color", m_Impl->m_WidgetBackgroundColor1[0]);
  preferences->Put("widget2 first background color", m_Impl->m_WidgetBackgroundColor1[1]);
  preferences->Put("widget3 first background color", m_Impl->m_WidgetBackgroundColor1[2]);
  preferences->Put("widget4 first background color", m_Impl->m_WidgetBackgroundColor1[3]);
  preferences->Put("widget1 second background color", m_Impl->m_WidgetBackgroundColor2[0]);
  preferences->Put("widget2 second background color", m_Impl->m_WidgetBackgroundColor2[1]);
  preferences->Put("widget3 second background color", m_Impl->m_WidgetBackgroundColor2[2]);
  preferences->Put("widget4 second background color", m_Impl->m_WidgetBackgroundColor2[3]);
}

void QmitkCustomMultiWidgetEditor::FillMembersWithCurrentDecorations()
{
  //fill members with current values (or default values) from the std multi widget
  for(unsigned int i = 0; i < 4; ++i)
  {
    m_Impl->m_WidgetDecorationColor[i] = MitkColorToHexColor(m_Impl->m_CustomMultiWidget->GetDecorationColor(i));
    m_Impl->m_WidgetBackgroundColor1[i] = MitkColorToHexColor(m_Impl->m_CustomMultiWidget->GetGradientColors(i).first);
    m_Impl->m_WidgetBackgroundColor2[i] = MitkColorToHexColor(m_Impl->m_CustomMultiWidget->GetGradientColors(i).second);
    m_Impl->m_WidgetAnnotation[i] = QString::fromStdString(m_Impl->m_CustomMultiWidget->GetCornerAnnotationText(i));
  }
}

void QmitkCustomMultiWidgetEditor::GetPreferenceDecorations(const berry::IBerryPreferences * preferences)
{
  //overwrite members with values from the preferences, if they the preference is defined
  m_Impl->m_WidgetBackgroundColor1[0] = preferences->Get("widget1 first background color", m_Impl->m_WidgetBackgroundColor1[0]);
  m_Impl->m_WidgetBackgroundColor2[0] = preferences->Get("widget1 second background color", m_Impl->m_WidgetBackgroundColor2[0]);
  m_Impl->m_WidgetBackgroundColor1[1] = preferences->Get("widget2 first background color", m_Impl->m_WidgetBackgroundColor1[1]);
  m_Impl->m_WidgetBackgroundColor2[1] = preferences->Get("widget2 second background color", m_Impl->m_WidgetBackgroundColor2[1]);
  m_Impl->m_WidgetBackgroundColor1[2] = preferences->Get("widget3 first background color", m_Impl->m_WidgetBackgroundColor1[2]);
  m_Impl->m_WidgetBackgroundColor2[2] = preferences->Get("widget3 second background color", m_Impl->m_WidgetBackgroundColor2[2]);
  m_Impl->m_WidgetBackgroundColor1[3] = preferences->Get("widget4 first background color", m_Impl->m_WidgetBackgroundColor1[3]);
  m_Impl->m_WidgetBackgroundColor2[3] = preferences->Get("widget4 second background color", m_Impl->m_WidgetBackgroundColor2[3]);

  m_Impl->m_WidgetDecorationColor[0] = preferences->Get("widget1 decoration color", m_Impl->m_WidgetDecorationColor[0]);
  m_Impl->m_WidgetDecorationColor[1] = preferences->Get("widget2 decoration color", m_Impl->m_WidgetDecorationColor[1]);
  m_Impl->m_WidgetDecorationColor[2] = preferences->Get("widget3 decoration color", m_Impl->m_WidgetDecorationColor[2]);
  m_Impl->m_WidgetDecorationColor[3] = preferences->Get("widget4 decoration color", m_Impl->m_WidgetDecorationColor[3]);

  m_Impl->m_WidgetAnnotation[0] = preferences->Get("widget1 corner annotation", m_Impl->m_WidgetAnnotation[0]);
  m_Impl->m_WidgetAnnotation[1] = preferences->Get("widget2 corner annotation", m_Impl->m_WidgetAnnotation[1]);
  m_Impl->m_WidgetAnnotation[2] = preferences->Get("widget3 corner annotation", m_Impl->m_WidgetAnnotation[2]);
  m_Impl->m_WidgetAnnotation[3] = preferences->Get("widget4 corner annotation", m_Impl->m_WidgetAnnotation[3]);
}

mitk::Color QmitkCustomMultiWidgetEditor::HexColorToMitkColor(const QString& hexColor)
{
  QColor qColor(hexColor);
  mitk::Color returnColor;
  float colorMax = 255.0f;
  if (hexColor.isEmpty()) // default value
  {
    returnColor[0] = 1.0;
    returnColor[1] = 1.0;
    returnColor[2] = 1.0;
    MITK_ERROR << "Using default color for unknown hex string " << qPrintable(hexColor);
  }
  else
  {
    returnColor[0] = qColor.red() / colorMax;
    returnColor[1] = qColor.green() / colorMax;
    returnColor[2] = qColor.blue() / colorMax;
  }

  return returnColor;
}

QString QmitkCustomMultiWidgetEditor::MitkColorToHexColor(const mitk::Color& mitkColor)
{
  QColor returnColor;
  float colorMax = 255.0f;
  returnColor.setRed(static_cast<int>(mitkColor[0] * colorMax + 0.5));
  returnColor.setGreen(static_cast<int>(mitkColor[1] * colorMax + 0.5));
  returnColor.setBlue(static_cast<int>(mitkColor[2] * colorMax + 0.5));

  return returnColor.name();
}
