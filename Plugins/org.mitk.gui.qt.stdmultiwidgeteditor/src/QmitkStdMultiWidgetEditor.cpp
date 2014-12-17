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

#include "QmitkStdMultiWidgetEditor.h"

#include <berryUIException.h>
#include <berryIWorkbenchPage.h>
#include <berryIPreferencesService.h>
#include <berryIPartListener.h>

#include <QWidget>

#include <mitkColorProperty.h>
#include <mitkGlobalInteraction.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

#include <mitkDataStorageEditorInput.h>
#include <mitkIDataStorageService.h>

#include <QmitkMouseModeSwitcher.h>
#include <QmitkStdMultiWidget.h>
#include <berryIBerryPreferences.h>

#include <mbilogo.h>

class QmitkStdMultiWidgetEditorPrivate
{
public:

  QmitkStdMultiWidgetEditorPrivate();
  ~QmitkStdMultiWidgetEditorPrivate();

  QmitkStdMultiWidget* m_StdMultiWidget;
  QmitkMouseModeSwitcher* m_MouseModeToolbar;
  std::string m_FirstBackgroundColor;
  std::string m_SecondBackgroundColor;
  bool m_MenuWidgetsEnabled;
  berry::IPartListener::Pointer m_PartListener;

  QHash<QString, QmitkRenderWindow*> m_RenderWindows;
};

struct QmitkStdMultiWidgetPartListener : public berry::IPartListener
{
  berryObjectMacro(QmitkStdMultiWidgetPartListener)

  QmitkStdMultiWidgetPartListener(QmitkStdMultiWidgetEditorPrivate* dd)
    : d(dd)
  {}

  Events::Types GetPartEventTypes() const
  {
    return Events::CLOSED | Events::HIDDEN | Events::VISIBLE;
  }

  void PartClosed (berry::IWorkbenchPartReference::Pointer partRef)
  {
    if (partRef->GetId() == QmitkStdMultiWidgetEditor::EDITOR_ID)
    {
      QmitkStdMultiWidgetEditor::Pointer stdMultiWidgetEditor = partRef->GetPart(false).Cast<QmitkStdMultiWidgetEditor>();

      if (d->m_StdMultiWidget == stdMultiWidgetEditor->GetStdMultiWidget())
      {
        d->m_StdMultiWidget->RemovePlanesFromDataStorage();
        stdMultiWidgetEditor->RequestActivateMenuWidget(false);
      }
    }
  }

  void PartHidden (berry::IWorkbenchPartReference::Pointer partRef)
  {
    if (partRef->GetId() == QmitkStdMultiWidgetEditor::EDITOR_ID)
    {
      QmitkStdMultiWidgetEditor::Pointer stdMultiWidgetEditor = partRef->GetPart(false).Cast<QmitkStdMultiWidgetEditor>();

      if (d->m_StdMultiWidget == stdMultiWidgetEditor->GetStdMultiWidget())
      {
        d->m_StdMultiWidget->RemovePlanesFromDataStorage();
        stdMultiWidgetEditor->RequestActivateMenuWidget(false);
      }
    }
  }

  void PartVisible (berry::IWorkbenchPartReference::Pointer partRef)
  {
    if (partRef->GetId() == QmitkStdMultiWidgetEditor::EDITOR_ID)
    {
      QmitkStdMultiWidgetEditor::Pointer stdMultiWidgetEditor = partRef->GetPart(false).Cast<QmitkStdMultiWidgetEditor>();

      if (d->m_StdMultiWidget == stdMultiWidgetEditor->GetStdMultiWidget())
      {
        d->m_StdMultiWidget->AddPlanesToDataStorage();
        stdMultiWidgetEditor->RequestActivateMenuWidget(true);
      }
    }
  }

private:

  QmitkStdMultiWidgetEditorPrivate* const d;
};

QmitkStdMultiWidgetEditorPrivate::QmitkStdMultiWidgetEditorPrivate()
  : m_StdMultiWidget(0), m_MouseModeToolbar(0)
  , m_MenuWidgetsEnabled(false)
  , m_PartListener(new QmitkStdMultiWidgetPartListener(this))
{}

QmitkStdMultiWidgetEditorPrivate::~QmitkStdMultiWidgetEditorPrivate()
{
}

const std::string QmitkStdMultiWidgetEditor::EDITOR_ID = "org.mitk.editors.stdmultiwidget";

QmitkStdMultiWidgetEditor::QmitkStdMultiWidgetEditor()
  : d(new QmitkStdMultiWidgetEditorPrivate)
{
}

QmitkStdMultiWidgetEditor::~QmitkStdMultiWidgetEditor()
{
  this->GetSite()->GetPage()->RemovePartListener(d->m_PartListener);
}

QmitkStdMultiWidget* QmitkStdMultiWidgetEditor::GetStdMultiWidget()
{
  return d->m_StdMultiWidget;
}

QmitkRenderWindow *QmitkStdMultiWidgetEditor::GetActiveQmitkRenderWindow() const
{
  if (d->m_StdMultiWidget) return d->m_StdMultiWidget->GetRenderWindow1();
  return 0;
}

QHash<QString, QmitkRenderWindow *> QmitkStdMultiWidgetEditor::GetQmitkRenderWindows() const
{
  return d->m_RenderWindows;
}

QmitkRenderWindow *QmitkStdMultiWidgetEditor::GetQmitkRenderWindow(const QString &id) const
{
  if (d->m_RenderWindows.contains(id))
    return d->m_RenderWindows[id];

  return 0;
}

mitk::Point3D QmitkStdMultiWidgetEditor::GetSelectedPosition(const QString & /*id*/) const
{
  return d->m_StdMultiWidget->GetCrossPosition();
}

void QmitkStdMultiWidgetEditor::SetSelectedPosition(const mitk::Point3D &pos, const QString &/*id*/)
{
  d->m_StdMultiWidget->MoveCrossToPosition(pos);
}

void QmitkStdMultiWidgetEditor::EnableDecorations(bool enable, const QStringList &decorations)
{
  if (decorations.isEmpty() || decorations.contains(DECORATION_BORDER))
  {
    enable ? d->m_StdMultiWidget->EnableColoredRectangles()
           : d->m_StdMultiWidget->DisableColoredRectangles();
  }
  if (decorations.isEmpty() || decorations.contains(DECORATION_LOGO))
  {
    enable ? d->m_StdMultiWidget->EnableDepartmentLogo()
           : d->m_StdMultiWidget->DisableDepartmentLogo();
  }
  if (decorations.isEmpty() || decorations.contains(DECORATION_MENU))
  {
    d->m_StdMultiWidget->ActivateMenuWidget(enable);
  }
  if (decorations.isEmpty() || decorations.contains(DECORATION_BACKGROUND))
  {
    enable ? d->m_StdMultiWidget->EnableGradientBackground()
           : d->m_StdMultiWidget->DisableGradientBackground();
  }
}

bool QmitkStdMultiWidgetEditor::IsDecorationEnabled(const QString &decoration) const
{
  if (decoration == DECORATION_BORDER)
  {
    return d->m_StdMultiWidget->IsColoredRectanglesEnabled();
  }
  else if (decoration == DECORATION_LOGO)
  {
    return d->m_StdMultiWidget->IsColoredRectanglesEnabled();
  }
  else if (decoration == DECORATION_MENU)
  {
    return d->m_StdMultiWidget->IsMenuWidgetEnabled();
  }
  else if (decoration == DECORATION_BACKGROUND)
  {
    return d->m_StdMultiWidget->GetGradientBackgroundFlag();
  }
  return false;
}

QStringList QmitkStdMultiWidgetEditor::GetDecorations() const
{
  QStringList decorations;
  decorations << DECORATION_BORDER << DECORATION_LOGO << DECORATION_MENU << DECORATION_BACKGROUND;
  return decorations;
}

mitk::SlicesRotator* QmitkStdMultiWidgetEditor::GetSlicesRotator() const
{
  return d->m_StdMultiWidget->GetSlicesRotator();
}

mitk::SlicesSwiveller* QmitkStdMultiWidgetEditor::GetSlicesSwiveller() const
{
  return d->m_StdMultiWidget->GetSlicesSwiveller();
}

void QmitkStdMultiWidgetEditor::EnableSlicingPlanes(bool enable)
{
  d->m_StdMultiWidget->SetWidgetPlanesVisibility(enable);
}

bool QmitkStdMultiWidgetEditor::IsSlicingPlanesEnabled() const
{
  mitk::DataNode::Pointer node = this->d->m_StdMultiWidget->GetWidgetPlane1();
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

void QmitkStdMultiWidgetEditor::EnableLinkedNavigation(bool enable)
{
  enable ? d->m_StdMultiWidget->EnableNavigationControllerEventListening()
         : d->m_StdMultiWidget->DisableNavigationControllerEventListening();
}

bool QmitkStdMultiWidgetEditor::IsLinkedNavigationEnabled() const
{
  return d->m_StdMultiWidget->IsCrosshairNavigationEnabled();
}

void QmitkStdMultiWidgetEditor::CreateQtPartControl(QWidget* parent)
{
  if (d->m_StdMultiWidget == 0)
  {
    QHBoxLayout* layout = new QHBoxLayout(parent);
    layout->setContentsMargins(0,0,0,0);

    if (d->m_MouseModeToolbar == NULL)
    {
      d->m_MouseModeToolbar = new QmitkMouseModeSwitcher(parent); // delete by Qt via parent
      layout->addWidget(d->m_MouseModeToolbar);
    }

    berry::IPreferences::Pointer prefs = this->GetPreferences();

    mitk::BaseRenderer::RenderingMode::Type renderingMode = static_cast<mitk::BaseRenderer::RenderingMode::Type>(prefs->GetInt( "Rendering Mode" , 0 ));

    d->m_StdMultiWidget = new QmitkStdMultiWidget(parent,0,0,renderingMode);

    d->m_RenderWindows.insert("axial", d->m_StdMultiWidget->GetRenderWindow1());
    d->m_RenderWindows.insert("sagittal", d->m_StdMultiWidget->GetRenderWindow2());
    d->m_RenderWindows.insert("coronal", d->m_StdMultiWidget->GetRenderWindow3());
    d->m_RenderWindows.insert("3d", d->m_StdMultiWidget->GetRenderWindow4());

    d->m_MouseModeToolbar->setMouseModeSwitcher( d->m_StdMultiWidget->GetMouseModeSwitcher() );
    connect( d->m_MouseModeToolbar, SIGNAL( MouseModeSelected(mitk::MouseModeSwitcher::MouseMode) ),
             d->m_StdMultiWidget, SLOT( MouseModeSelected(mitk::MouseModeSwitcher::MouseMode) ) );

    layout->addWidget(d->m_StdMultiWidget);

    mitk::DataStorage::Pointer ds = this->GetDataStorage();

    // Tell the multiWidget which (part of) the tree to render
    d->m_StdMultiWidget->SetDataStorage(ds);

    // Initialize views as axial, sagittal, coronar to all data objects in DataStorage
    // (from top-left to bottom)
    mitk::TimeGeometry::Pointer geo = ds->ComputeBoundingGeometry3D(ds->GetAll());
    mitk::RenderingManager::GetInstance()->InitializeViews(geo);

    // Initialize bottom-right view as 3D view
    d->m_StdMultiWidget->GetRenderWindow4()->GetRenderer()->SetMapperID(
          mitk::BaseRenderer::Standard3D );

    // Enable standard handler for levelwindow-slider
    d->m_StdMultiWidget->EnableStandardLevelWindow();

    // Add the displayed views to the tree to see their positions
    // in 2D and 3D
    d->m_StdMultiWidget->AddDisplayPlaneSubTree();

    d->m_StdMultiWidget->EnableNavigationControllerEventListening();

    // Store the initial visibility status of the menu widget.
    d->m_MenuWidgetsEnabled = d->m_StdMultiWidget->IsMenuWidgetEnabled();

    this->GetSite()->GetPage()->AddPartListener(d->m_PartListener);

    berry::IBerryPreferences* berryprefs = dynamic_cast<berry::IBerryPreferences*>(prefs.GetPointer());
    InitializePreferences(berryprefs);
    this->OnPreferencesChanged(berryprefs);

    this->RequestUpdate();
  }
}

void QmitkStdMultiWidgetEditor::OnPreferencesChanged(const berry::IBerryPreferences* prefs)
{
  // Enable change of logo. If no DepartmentLogo was set explicitly, MBILogo is used.
  // Set new department logo by prefs->Set("DepartmentLogo", "PathToImage");
  // If no logo was set for this plug-in specifically, walk the parent preference nodes
  // and lookup a logo value there.
  const berry::IPreferences* currentNode = prefs;

  while(currentNode)
  {
    std::vector<std::string> keys = currentNode->Keys();

    bool logoFound = false;
    for( std::size_t i = 0; i < keys.size(); ++i )
    {
      if( keys[i] == "DepartmentLogo")
      {
        std::string departmentLogoLocation = currentNode->Get("DepartmentLogo", "");

        if (departmentLogoLocation.empty())
        {
          d->m_StdMultiWidget->DisableDepartmentLogo();
        }
        else
        {
          // we need to disable the logo first, otherwise setting a new logo will have
          // no effect due to how mitkManufacturerLogo works...
          d->m_StdMultiWidget->DisableDepartmentLogo();
          d->m_StdMultiWidget->SetDepartmentLogoPath(departmentLogoLocation.c_str());
          d->m_StdMultiWidget->EnableDepartmentLogo();
        }
        logoFound = true;
        break;
      }
    }

    if (logoFound) break;
    currentNode = currentNode->Parent().GetPointer();
  }

  // preferences for gradient background
  mitk::Color upper = HexColorToMitkColor(prefs->GetByteArray("widget1 first background color", "#000000"));
  mitk::Color lower = HexColorToMitkColor(prefs->GetByteArray("widget1 second background color", "#000000"));
  d->m_StdMultiWidget->SetGradientBackgroundColorForRenderWindow(upper, lower, 0);
  upper = HexColorToMitkColor(prefs->GetByteArray("widget2 first background color", "#000000"));
  lower = HexColorToMitkColor(prefs->GetByteArray("widget2 second background color", "#000000"));
  d->m_StdMultiWidget->SetGradientBackgroundColorForRenderWindow(upper, lower, 1);
  upper = HexColorToMitkColor(prefs->GetByteArray("widget3 first background color", "#000000"));
  lower = HexColorToMitkColor(prefs->GetByteArray("widget3 second background color", "#000000"));
  d->m_StdMultiWidget->SetGradientBackgroundColorForRenderWindow(upper, lower, 2);
  upper = HexColorToMitkColor(prefs->GetByteArray("widget4 first background color", "#191919"));
  lower = HexColorToMitkColor(prefs->GetByteArray("widget4 second background color", "#7F7F7F"));
  d->m_StdMultiWidget->SetGradientBackgroundColorForRenderWindow(upper, lower, 3);
  d->m_StdMultiWidget->EnableGradientBackground();

  // preferences for renderWindows
  mitk::Color colorDecorationWidget1 = HexColorToMitkColor(prefs->GetByteArray("widget1 decoration color", "#FF0000"));
  mitk::Color colorDecorationWidget2 = HexColorToMitkColor(prefs->GetByteArray("widget2 decoration color", "#00FF00"));
  mitk::Color colorDecorationWidget3 = HexColorToMitkColor(prefs->GetByteArray("widget3 decoration color", "#0000FF"));
  mitk::Color colorDecorationWidget4 = HexColorToMitkColor(prefs->GetByteArray("widget4 decoration color", "#FFFF00"));
  mitk::BaseRenderer::GetInstance(d->m_StdMultiWidget->GetRenderWindow1()->GetVtkRenderWindow())
      ->GetCurrentWorldPlaneGeometryNode()->SetColor(colorDecorationWidget1);
  mitk::BaseRenderer::GetInstance(d->m_StdMultiWidget->GetRenderWindow2()->GetVtkRenderWindow())
      ->GetCurrentWorldPlaneGeometryNode()->SetColor(colorDecorationWidget2);
  mitk::BaseRenderer::GetInstance(d->m_StdMultiWidget->GetRenderWindow3()->GetVtkRenderWindow())
      ->GetCurrentWorldPlaneGeometryNode()->SetColor(colorDecorationWidget3);
  d->m_StdMultiWidget->SetDecorationColorWidget4(colorDecorationWidget4);

  //The crosshair gap
  int crosshairgapsize = prefs->GetInt("crosshair gap size", 32);
  mitk::BaseRenderer::GetInstance(d->m_StdMultiWidget->GetRenderWindow1()->GetVtkRenderWindow())
      ->GetCurrentWorldPlaneGeometryNode()->SetIntProperty("Crosshair.Gap Size", crosshairgapsize);
  mitk::BaseRenderer::GetInstance(d->m_StdMultiWidget->GetRenderWindow2()->GetVtkRenderWindow())
      ->GetCurrentWorldPlaneGeometryNode()->SetIntProperty("Crosshair.Gap Size", crosshairgapsize);
  mitk::BaseRenderer::GetInstance(d->m_StdMultiWidget->GetRenderWindow3()->GetVtkRenderWindow())
      ->GetCurrentWorldPlaneGeometryNode()->SetIntProperty("Crosshair.Gap Size", crosshairgapsize);

  //refresh colors of rectangles
  d->m_StdMultiWidget->EnableColoredRectangles();

  // Set preferences respecting zooming and padding
  bool constrainedZooming = prefs->GetBool("Use constrained zooming and padding", false);

  mitk::RenderingManager::GetInstance()->SetConstrainedPaddingZooming(constrainedZooming);

  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  // level window setting
  bool showLevelWindowWidget = prefs->GetBool("Show level/window widget", true);
  if (showLevelWindowWidget)
  {
    d->m_StdMultiWidget->EnableStandardLevelWindow();
  }
  else
  {
    d->m_StdMultiWidget->DisableStandardLevelWindow();
  }

  // mouse modes toolbar
  bool newMode = prefs->GetBool("PACS like mouse interaction", false);
  d->m_MouseModeToolbar->setVisible( newMode );
  d->m_StdMultiWidget->GetMouseModeSwitcher()->SetInteractionScheme( newMode ? mitk::MouseModeSwitcher::PACS : mitk::MouseModeSwitcher::MITK );

  d->m_StdMultiWidget->SetCornerAnnotation(
        prefs->GetByteArray("widget1 corner annotation", "Axial"), colorDecorationWidget1, 0);
  d->m_StdMultiWidget->SetCornerAnnotation(
        prefs->GetByteArray("widget2 corner annotation", "Sagittal"), colorDecorationWidget2, 1);
  d->m_StdMultiWidget->SetCornerAnnotation(
        prefs->GetByteArray("widget3 corner annotation", "Coronal"), colorDecorationWidget3, 2);
  d->m_StdMultiWidget->SetCornerAnnotation(
        prefs->GetByteArray("widget4 corner annotation", "3D"), colorDecorationWidget4, 3);
}

mitk::Color QmitkStdMultiWidgetEditor::HexColorToMitkColor(std::string widgetColorInHex)
{
  QString widgetColorQt = QString::fromStdString(widgetColorInHex);
  QColor qColor(widgetColorQt);
  mitk::Color returnColor;
  float colorMax = 255.0f;
  if (widgetColorQt=="") // default value
  {
    returnColor[0] = 1.0;
    returnColor[1] = 1.0;
    returnColor[2] = 1.0;
    MITK_ERROR << "Using default color for unknown widget " << widgetColorInHex;
  }
  else
  {
    returnColor[0] = qColor.red() / colorMax;
    returnColor[1] = qColor.green() / colorMax;
    returnColor[2] = qColor.blue() / colorMax;
  }
  return returnColor;
}

void QmitkStdMultiWidgetEditor::InitializePreferences(berry::IBerryPreferences * preferences)
{
  std::string widgetBackgroundColor1[4];
  std::string widgetBackgroundColor2[4];
  std::string widgetDecorationColor[4];
  std::string widgetAnnotation[4];
  widgetBackgroundColor1[0] = preferences->GetByteArray("widget1 first background color", "#000000");
  widgetBackgroundColor2[0] = preferences->GetByteArray("widget1 second background color", "#000000");
  widgetBackgroundColor1[1] = preferences->GetByteArray("widget2 first background color", "#000000");
  widgetBackgroundColor2[1] = preferences->GetByteArray("widget2 second background color", "#000000");
  widgetBackgroundColor1[2] = preferences->GetByteArray("widget3 first background color", "#000000");
  widgetBackgroundColor2[2] = preferences->GetByteArray("widget3 second background color", "#000000");
  widgetBackgroundColor1[3] = preferences->GetByteArray("widget4 first background color", "#191919");
  widgetBackgroundColor2[3] = preferences->GetByteArray("widget4 second background color", "#7F7F7F");

  //decoration colors
  widgetDecorationColor[0] = preferences->GetByteArray("widget1 decoration color", "#C00000");
  widgetDecorationColor[1] = preferences->GetByteArray("widget2 decoration color", "#00B000");
  widgetDecorationColor[2] = preferences->GetByteArray("widget3 decoration color", "#0080FF");
  widgetDecorationColor[3] = preferences->GetByteArray("widget4 decoration color", "#FFFF00");

  //annotation text
  widgetAnnotation[0] = preferences->GetByteArray("widget1 corner annotation", "Axial");
  widgetAnnotation[1] = preferences->GetByteArray("widget2 corner annotation", "Sagittal");
  widgetAnnotation[2] = preferences->GetByteArray("widget3 corner annotation", "Coronal");
  widgetAnnotation[3] = preferences->GetByteArray("widget4 corner annotation", "3D");

  preferences->PutByteArray("widget1 corner annotation", widgetAnnotation[0]);
  preferences->PutByteArray("widget2 corner annotation", widgetAnnotation[1]);
  preferences->PutByteArray("widget3 corner annotation", widgetAnnotation[2]);
  preferences->PutByteArray("widget4 corner annotation", widgetAnnotation[3]);

  preferences->PutByteArray("widget1 decoration color", widgetDecorationColor[0]);
  preferences->PutByteArray("widget2 decoration color", widgetDecorationColor[1]);
  preferences->PutByteArray("widget3 decoration color", widgetDecorationColor[2]);
  preferences->PutByteArray("widget4 decoration color", widgetDecorationColor[3]);

  preferences->PutByteArray("widget1 first background color", widgetBackgroundColor1[0]);
  preferences->PutByteArray("widget2 first background color", widgetBackgroundColor1[1]);
  preferences->PutByteArray("widget3 first background color", widgetBackgroundColor1[2]);
  preferences->PutByteArray("widget4 first background color", widgetBackgroundColor1[3]);
  preferences->PutByteArray("widget1 second background color", widgetBackgroundColor2[0]);
  preferences->PutByteArray("widget2 second background color", widgetBackgroundColor2[1]);
  preferences->PutByteArray("widget3 second background color", widgetBackgroundColor2[2]);
  preferences->PutByteArray("widget4 second background color", widgetBackgroundColor2[3]);
}

void QmitkStdMultiWidgetEditor::SetFocus()
{
  if (d->m_StdMultiWidget != 0)
    d->m_StdMultiWidget->setFocus();
}

void QmitkStdMultiWidgetEditor::RequestActivateMenuWidget(bool on)
{
  if (d->m_StdMultiWidget)
  {
    if (on)
    {
      d->m_StdMultiWidget->ActivateMenuWidget(d->m_MenuWidgetsEnabled);
    }
    else
    {
      d->m_MenuWidgetsEnabled = d->m_StdMultiWidget->IsMenuWidgetEnabled();
      d->m_StdMultiWidget->ActivateMenuWidget(false);
    }
  }
}
