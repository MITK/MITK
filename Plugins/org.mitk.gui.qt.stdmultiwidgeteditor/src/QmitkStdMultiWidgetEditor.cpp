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
#include <berryIPreferences.h>

#include <QWidget>

#include <mitkColorProperty.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

#include <mitkDataStorageEditorInput.h>
#include <mitkIDataStorageService.h>

#include <QmitkMouseModeSwitcher.h>
#include <QmitkStdMultiWidget.h>

#include <mbilogo.h>

class QmitkStdMultiWidgetEditorPrivate
{
public:

  QmitkStdMultiWidgetEditorPrivate();
  ~QmitkStdMultiWidgetEditorPrivate();

  QmitkStdMultiWidget* m_StdMultiWidget;
  QmitkMouseModeSwitcher* m_MouseModeToolbar;
  /**
  * @brief Members for the MultiWidget decorations.
  */
  QString m_WidgetBackgroundColor1[4];
  QString m_WidgetBackgroundColor2[4];
  QString m_WidgetDecorationColor[4];
  QString m_WidgetAnnotation[4];
  bool m_MenuWidgetsEnabled;
  QScopedPointer<berry::IPartListener> m_PartListener;

  QHash<QString, QmitkRenderWindow*> m_RenderWindows;

};

struct QmitkStdMultiWidgetPartListener : public berry::IPartListener
{
  QmitkStdMultiWidgetPartListener(QmitkStdMultiWidgetEditorPrivate* dd)
    : d(dd)
  {}

  Events::Types GetPartEventTypes() const override
  {
    return Events::CLOSED | Events::HIDDEN | Events::VISIBLE;
  }

  void PartClosed(const berry::IWorkbenchPartReference::Pointer& partRef) override
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

  void PartHidden(const berry::IWorkbenchPartReference::Pointer& partRef) override
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

  void PartVisible(const berry::IWorkbenchPartReference::Pointer& partRef) override
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

const QString QmitkStdMultiWidgetEditor::EDITOR_ID = "org.mitk.editors.stdmultiwidget";

QmitkStdMultiWidgetEditor::QmitkStdMultiWidgetEditor()
  : d(new QmitkStdMultiWidgetEditorPrivate)
{
}

QmitkStdMultiWidgetEditor::~QmitkStdMultiWidgetEditor()
{
  this->GetSite()->GetPage()->RemovePartListener(d->m_PartListener.data());
}

void QmitkStdMultiWidgetEditor::SetWindowPresetBeforeLoading()
{
  d->m_StdMultiWidget->changeLayoutToWidget1();
}

void QmitkStdMultiWidgetEditor::SetWindowPresetAfterLoading()
{
  d->m_StdMultiWidget->changeLayoutToDefault();
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
  if (decorations.isEmpty() || decorations.contains(DECORATION_CORNER_ANNOTATION))
  {
    enable ? d->m_StdMultiWidget->SetCornerAnnotationVisibility(true)
           : d->m_StdMultiWidget->SetCornerAnnotationVisibility(false);
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
  else if (decoration == DECORATION_CORNER_ANNOTATION)
  {
    return d->m_StdMultiWidget->IsCornerAnnotationVisible();
  }

  return false;
}

QStringList QmitkStdMultiWidgetEditor::GetDecorations() const
{
  QStringList decorations;
  decorations << DECORATION_BORDER << DECORATION_LOGO << DECORATION_MENU << DECORATION_BACKGROUND << DECORATION_CORNER_ANNOTATION;
  return decorations;
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

    //d->m_StdMultiWidget->EnableNavigationControllerEventListening();

    // Store the initial visibility status of the menu widget.
    d->m_MenuWidgetsEnabled = d->m_StdMultiWidget->IsMenuWidgetEnabled();

    this->GetSite()->GetPage()->AddPartListener(d->m_PartListener.data());

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
    bool logoFound = false;
    foreach (const QString& key, currentNode->Keys())
    {
      if( key == "DepartmentLogo")
      {
        QString departmentLogoLocation = currentNode->Get("DepartmentLogo", "");

        if (departmentLogoLocation.isEmpty())
        {
          d->m_StdMultiWidget->DisableDepartmentLogo();
        }
        else
        {
          // we need to disable the logo first, otherwise setting a new logo will have
          // no effect due to how mitkManufacturerLogo works...
          d->m_StdMultiWidget->DisableDepartmentLogo();
          d->m_StdMultiWidget->SetDepartmentLogoPath(qPrintable(departmentLogoLocation));
          d->m_StdMultiWidget->EnableDepartmentLogo();
        }
        logoFound = true;
        break;
      }
    }

    if (logoFound) break;
    currentNode = currentNode->Parent().GetPointer();
  }

  //Update internal members
  this->FillMembersWithCurrentDecorations();
  this->GetPreferenceDecorations(prefs);
  //Now the members can be used to modify the stdmultiwidget
  mitk::Color upper = HexColorToMitkColor(d->m_WidgetBackgroundColor1[0]);
  mitk::Color lower = HexColorToMitkColor(d->m_WidgetBackgroundColor2[0]);
  d->m_StdMultiWidget->SetGradientBackgroundColorForRenderWindow(upper, lower, 0);
  upper = HexColorToMitkColor(d->m_WidgetBackgroundColor1[1]);
  lower = HexColorToMitkColor(d->m_WidgetBackgroundColor2[1]);
  d->m_StdMultiWidget->SetGradientBackgroundColorForRenderWindow(upper, lower, 1);
  upper = HexColorToMitkColor(d->m_WidgetBackgroundColor1[2]);
  lower = HexColorToMitkColor(d->m_WidgetBackgroundColor2[2]);
  d->m_StdMultiWidget->SetGradientBackgroundColorForRenderWindow(upper, lower, 2);
  upper = HexColorToMitkColor(d->m_WidgetBackgroundColor1[3]);
  lower = HexColorToMitkColor(d->m_WidgetBackgroundColor2[3]);
  d->m_StdMultiWidget->SetGradientBackgroundColorForRenderWindow(upper, lower, 3);
  d->m_StdMultiWidget->EnableGradientBackground();

  // preferences for renderWindows
  mitk::Color colorDecorationWidget1 = HexColorToMitkColor(d->m_WidgetDecorationColor[0]);
  mitk::Color colorDecorationWidget2 = HexColorToMitkColor(d->m_WidgetDecorationColor[1]);
  mitk::Color colorDecorationWidget3 = HexColorToMitkColor(d->m_WidgetDecorationColor[2]);
  mitk::Color colorDecorationWidget4 = HexColorToMitkColor(d->m_WidgetDecorationColor[3]);
  d->m_StdMultiWidget->SetDecorationColor(0, colorDecorationWidget1);
  d->m_StdMultiWidget->SetDecorationColor(1, colorDecorationWidget2);
  d->m_StdMultiWidget->SetDecorationColor(2, colorDecorationWidget3);
  d->m_StdMultiWidget->SetDecorationColor(3, colorDecorationWidget4);

  for(unsigned int i = 0; i < 4; ++i)
  {
    d->m_StdMultiWidget->SetDecorationProperties(d->m_WidgetAnnotation[i].toStdString(),
                                             HexColorToMitkColor(d->m_WidgetDecorationColor[i]), i);
  }
  //The crosshair gap
  int crosshairgapsize = prefs->GetInt("crosshair gap size", 32);
  d->m_StdMultiWidget->GetWidgetPlane1()->SetIntProperty("Crosshair.Gap Size", crosshairgapsize);
  d->m_StdMultiWidget->GetWidgetPlane2()->SetIntProperty("Crosshair.Gap Size", crosshairgapsize);
  d->m_StdMultiWidget->GetWidgetPlane3()->SetIntProperty("Crosshair.Gap Size", crosshairgapsize);

  //refresh colors of rectangles
  d->m_StdMultiWidget->EnableColoredRectangles();

  // Set preferences respecting zooming and panning
  bool constrainedZooming = prefs->GetBool("Use constrained zooming and panning", true);

  mitk::RenderingManager::GetInstance()->SetConstrainedPanningZooming(constrainedZooming);

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

  bool displayMetainfo = prefs->GetBool("Display metainfo", true);
  d->m_StdMultiWidget->setDisplayMetaInfo( displayMetainfo );

  bool selectionMode = prefs->GetBool("Selection on 3D view", false);
  d->m_StdMultiWidget->setSelectionMode(selectionMode);

  // mouse modes toolbar
  bool newMode = prefs->GetBool("PACS like mouse interaction", false);
  d->m_MouseModeToolbar->setVisible( newMode );
  d->m_StdMultiWidget->GetMouseModeSwitcher()->SetInteractionScheme( newMode ? mitk::MouseModeSwitcher::PACS : mitk::MouseModeSwitcher::MITK );
}

mitk::Color QmitkStdMultiWidgetEditor::HexColorToMitkColor(const QString& widgetColorInHex)
{
  QColor qColor(widgetColorInHex);
  mitk::Color returnColor;
  float colorMax = 255.0f;
  if (widgetColorInHex.isEmpty()) // default value
  {
    returnColor[0] = 1.0;
    returnColor[1] = 1.0;
    returnColor[2] = 1.0;
    MITK_ERROR << "Using default color for unknown widget " << qPrintable(widgetColorInHex);
  }
  else
  {
    returnColor[0] = qColor.red() / colorMax;
    returnColor[1] = qColor.green() / colorMax;
    returnColor[2] = qColor.blue() / colorMax;
  }
  return returnColor;
}

QString QmitkStdMultiWidgetEditor::MitkColorToHex(const mitk::Color& color)
{
  QColor returnColor;
  float colorMax = 255.0f;
  returnColor.setRed(static_cast<int>(color[0]* colorMax + 0.5));
  returnColor.setGreen(static_cast<int>(color[1]* colorMax + 0.5));
  returnColor.setBlue(static_cast<int>(color[2]* colorMax + 0.5));
  return returnColor.name();
}

void QmitkStdMultiWidgetEditor::FillMembersWithCurrentDecorations()
{
  //fill members with current values (or default values) from the std multi widget
  for(unsigned int i = 0; i < 4; ++i)
  {
    d->m_WidgetDecorationColor[i] = MitkColorToHex(d->m_StdMultiWidget->GetDecorationColor(i));
    d->m_WidgetBackgroundColor1[i] = MitkColorToHex(d->m_StdMultiWidget->GetGradientColors(i).first);
    d->m_WidgetBackgroundColor2[i] = MitkColorToHex(d->m_StdMultiWidget->GetGradientColors(i).second);
    d->m_WidgetAnnotation[i] = QString::fromStdString(d->m_StdMultiWidget->GetCornerAnnotationText(i));
  }
}

void QmitkStdMultiWidgetEditor::GetPreferenceDecorations(const berry::IBerryPreferences * preferences)
{
  //overwrite members with values from the preferences, if they the prefrence is defined
  d->m_WidgetBackgroundColor1[0] = preferences->Get("widget1 first background color", d->m_WidgetBackgroundColor1[0]);
  d->m_WidgetBackgroundColor2[0] = preferences->Get("widget1 second background color", d->m_WidgetBackgroundColor2[0]);
  d->m_WidgetBackgroundColor1[1] = preferences->Get("widget2 first background color", d->m_WidgetBackgroundColor1[1]);
  d->m_WidgetBackgroundColor2[1] = preferences->Get("widget2 second background color", d->m_WidgetBackgroundColor2[1]);
  d->m_WidgetBackgroundColor1[2] = preferences->Get("widget3 first background color", d->m_WidgetBackgroundColor1[2]);
  d->m_WidgetBackgroundColor2[2] = preferences->Get("widget3 second background color", d->m_WidgetBackgroundColor2[2]);
  d->m_WidgetBackgroundColor1[3] = preferences->Get("widget4 first background color", d->m_WidgetBackgroundColor1[3]);
  d->m_WidgetBackgroundColor2[3] = preferences->Get("widget4 second background color", d->m_WidgetBackgroundColor2[3]);

  d->m_WidgetDecorationColor[0] = preferences->Get("widget1 decoration color", d->m_WidgetDecorationColor[0]);
  d->m_WidgetDecorationColor[1] = preferences->Get("widget2 decoration color", d->m_WidgetDecorationColor[1]);
  d->m_WidgetDecorationColor[2] = preferences->Get("widget3 decoration color", d->m_WidgetDecorationColor[2]);
  d->m_WidgetDecorationColor[3] = preferences->Get("widget4 decoration color", d->m_WidgetDecorationColor[3]);

  d->m_WidgetAnnotation[0] = preferences->Get("widget1 corner annotation", d->m_WidgetAnnotation[0]);
  d->m_WidgetAnnotation[1] = preferences->Get("widget2 corner annotation", d->m_WidgetAnnotation[1]);
  d->m_WidgetAnnotation[2] = preferences->Get("widget3 corner annotation", d->m_WidgetAnnotation[2]);
  d->m_WidgetAnnotation[3] = preferences->Get("widget4 corner annotation", d->m_WidgetAnnotation[3]);
}

void QmitkStdMultiWidgetEditor::InitializePreferences(berry::IBerryPreferences * preferences)
{
  this->FillMembersWithCurrentDecorations(); //fill members
  this->GetPreferenceDecorations(preferences); //overwrite if preferences are defined

  //create new preferences
  preferences->Put("widget1 corner annotation", d->m_WidgetAnnotation[0]);
  preferences->Put("widget2 corner annotation", d->m_WidgetAnnotation[1]);
  preferences->Put("widget3 corner annotation", d->m_WidgetAnnotation[2]);
  preferences->Put("widget4 corner annotation", d->m_WidgetAnnotation[3]);

  preferences->Put("widget1 decoration color", d->m_WidgetDecorationColor[0]);
  preferences->Put("widget2 decoration color", d->m_WidgetDecorationColor[1]);
  preferences->Put("widget3 decoration color", d->m_WidgetDecorationColor[2]);
  preferences->Put("widget4 decoration color", d->m_WidgetDecorationColor[3]);

  preferences->Put("widget1 first background color", d->m_WidgetBackgroundColor1[0]);
  preferences->Put("widget2 first background color", d->m_WidgetBackgroundColor1[1]);
  preferences->Put("widget3 first background color", d->m_WidgetBackgroundColor1[2]);
  preferences->Put("widget4 first background color", d->m_WidgetBackgroundColor1[3]);
  preferences->Put("widget1 second background color", d->m_WidgetBackgroundColor2[0]);
  preferences->Put("widget2 second background color", d->m_WidgetBackgroundColor2[1]);
  preferences->Put("widget3 second background color", d->m_WidgetBackgroundColor2[2]);
  preferences->Put("widget4 second background color", d->m_WidgetBackgroundColor2[3]);
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

