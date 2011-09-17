/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkStdMultiWidgetEditor.h"

#include <berryUIException.h>
#include <berryIWorkbenchPage.h>
#include <berryIPreferencesService.h>

#include <QWidget>

#include <mitkColorProperty.h>
#include <mitkGlobalInteraction.h>

#include <mitkDataStorageEditorInput.h>
#include <mitkIDataStorageService.h>

#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateProperty.h"

const std::string QmitkStdMultiWidgetEditor::EDITOR_ID = "org.mitk.editors.stdmultiwidget";

QmitkStdMultiWidgetEditor::QmitkStdMultiWidgetEditor()
 : m_StdMultiWidget(0)
{

}

QmitkStdMultiWidgetEditor::QmitkStdMultiWidgetEditor(const QmitkStdMultiWidgetEditor& other)
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}

QmitkStdMultiWidgetEditor::~QmitkStdMultiWidgetEditor()
{
  // we need to wrap the RemovePartListener call inside a
  // register/unregister block to prevent infinite recursion
  // due to the destruction of temporary smartpointer to this
  this->Register();
  this->GetSite()->GetPage()->RemovePartListener(berry::IPartListener::Pointer(this));
  this->UnRegister(false);
}

QmitkStdMultiWidget* QmitkStdMultiWidgetEditor::GetStdMultiWidget()
{
  return m_StdMultiWidget;
}

void QmitkStdMultiWidgetEditor::Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input)
{
  if (input.Cast<mitk::DataStorageEditorInput>().IsNull())
     throw berry::PartInitException("Invalid Input: Must be IFileEditorInput");

  this->SetSite(site);
  this->SetInput(input);
}

void QmitkStdMultiWidgetEditor::CreateQtPartControl(QWidget* parent)
{
  if (m_StdMultiWidget == 0)
  {
    m_DndFrameWidget = new QmitkDnDFrameWidget(parent);
    QVBoxLayout* layout = new QVBoxLayout(parent);
    layout->addWidget(m_DndFrameWidget);
    layout->setContentsMargins(0,0,0,0); 

    m_StdMultiWidget = new QmitkStdMultiWidget(m_DndFrameWidget);
    QVBoxLayout* layout2 = new QVBoxLayout(m_DndFrameWidget);
    layout2->addWidget(m_StdMultiWidget);
    layout2->setContentsMargins(0,0,0,0); 

    mitk::DataStorage::Pointer ds = this->GetEditorInput().Cast<mitk::DataStorageEditorInput>()
      ->GetDataStorageReference()->GetDataStorage();

    // Tell the multiWidget which (part of) the tree to render
    m_StdMultiWidget->SetDataStorage(ds);

    // Initialize views as transversal, sagittal, coronar to all data objects in DataStorage
    // (from top-left to bottom)
    mitk::TimeSlicedGeometry::Pointer geo = ds->ComputeBoundingGeometry3D(ds->GetAll());
    mitk::RenderingManager::GetInstance()->InitializeViews(geo);

    // Initialize bottom-right view as 3D view
    m_StdMultiWidget->GetRenderWindow4()->GetRenderer()->SetMapperID(
      mitk::BaseRenderer::Standard3D );

    // Enable standard handler for levelwindow-slider
    m_StdMultiWidget->EnableStandardLevelWindow();

    // Add the displayed views to the tree to see their positions
    // in 2D and 3D
    m_StdMultiWidget->AddDisplayPlaneSubTree();

    m_StdMultiWidget->EnableNavigationControllerEventListening();

    mitk::GlobalInteraction::GetInstance()->AddListener(
        m_StdMultiWidget->GetMoveAndZoomInteractor()
      );
    this->GetSite()->GetPage()->AddPartListener(berry::IPartListener::Pointer(this));

    // enable change of logo
    berry::IPreferencesService::Pointer prefService
      = berry::Platform::GetServiceRegistry()
      .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

    berry::IPreferences::Pointer logoPref = prefService->GetSystemPreferences()->Node("DepartmentLogo");
    std::string departmentLogoLocation = logoPref->Get("DepartmentLogo","");

    //# Preferences

    berry::IBerryPreferences::Pointer prefs
        = (prefService->GetSystemPreferences()->Node(EDITOR_ID))
          .Cast<berry::IBerryPreferences>();
    assert( prefs );

    prefs->OnChanged.AddListener( berry::MessageDelegate1<QmitkStdMultiWidgetEditor
      , const berry::IBerryPreferences*>( this
        , &QmitkStdMultiWidgetEditor::OnPreferencesChanged ) );

    bool constrainedZooming = prefs->GetBool("Use constrained zooming and padding", false);

    mitk::RenderingManager::GetInstance()->SetConstrainedPaddingZooming(constrainedZooming);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    bool showLevelWindowWidget = prefs->GetBool("Show level/window widget", true);
    // The widget is enabled after construction, so we check only if it has to be disabled.
    if (!showLevelWindowWidget)
    {
      m_StdMultiWidget->DisableStandardLevelWindow();
    }

    m_StdMultiWidget->SetDepartmentLogoPath(departmentLogoLocation.c_str());
    m_StdMultiWidget->DisableDepartmentLogo();
    m_StdMultiWidget->EnableDepartmentLogo();

    this->OnPreferencesChanged(prefs.GetPointer());
  }
}

void QmitkStdMultiWidgetEditor::OnPreferencesChanged(const berry::IBerryPreferences* prefs)
{
  // preferences for gradient background
  float color = 255.0;
  QString firstColorName = QString::fromStdString (prefs->GetByteArray("first background color", ""));
  QColor firstColor(firstColorName);
  mitk::Color upper;
  if (firstColorName=="") // default values
  {
    upper[0] = 0.1;
    upper[1] = 0.1;
    upper[2] = 0.1;
  }
  else
  {
    upper[0] = firstColor.red() / color;
    upper[1] = firstColor.green() / color;
    upper[2] = firstColor.blue() / color;
  }

  QString secondColorName = QString::fromStdString (prefs->GetByteArray("second background color", ""));
  QColor secondColor(secondColorName);
  mitk::Color lower;
  if (secondColorName=="") // default values
  {
    lower[0] = 0.5;
    lower[1] = 0.5;
    lower[2] = 0.5;
  }
  else
  {
    lower[0] = secondColor.red() / color;
    lower[1] = secondColor.green() / color;
    lower[2] = secondColor.blue() / color;
  }
  m_StdMultiWidget->SetGradientBackgroundColors(upper, lower);
  m_StdMultiWidget->EnableGradientBackground();

  // Set preferences respecting zooming and padding
  bool constrainedZooming = prefs->GetBool("Use constrained zooming and padding", false);

  mitk::RenderingManager::GetInstance()->SetConstrainedPaddingZooming(constrainedZooming);

  mitk::NodePredicateNot::Pointer pred
    = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("includeInBoundingBox"
    , mitk::BoolProperty::New(false)));

  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetDataStorage()->GetSubset(pred);
  // calculate bounding geometry of these nodes

  mitk::TimeSlicedGeometry::Pointer bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(rs, "visible");


  // initialize the views to the bounding geometry
  mitk::RenderingManager::GetInstance()->InitializeViews(bounds);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  bool showLevelWindowWidget = prefs->GetBool("Show level/window widget", true);
  if (showLevelWindowWidget)
  {
    m_StdMultiWidget->EnableStandardLevelWindow();
  }
  else
  {
    m_StdMultiWidget->DisableStandardLevelWindow();
  }
}


mitk::DataStorage::Pointer QmitkStdMultiWidgetEditor::GetDataStorage() const
{
  mitk::IDataStorageService::Pointer service =
    berry::Platform::GetServiceRegistry().GetServiceById<mitk::IDataStorageService>(mitk::IDataStorageService::ID);

  if (service.IsNotNull())
  {
    return service->GetDefaultDataStorage()->GetDataStorage();
  }

  return 0;
}


berry::IPartListener::Events::Types QmitkStdMultiWidgetEditor::GetPartEventTypes() const
{
  return Events::CLOSED | Events::HIDDEN | Events::VISIBLE;
}

void QmitkStdMultiWidgetEditor::PartClosed( berry::IWorkbenchPartReference::Pointer partRef )
{
  if (partRef->GetId() == QmitkStdMultiWidgetEditor::EDITOR_ID)
  {
    QmitkStdMultiWidgetEditor::Pointer stdMultiWidgetEditor = partRef->GetPart(false).Cast<QmitkStdMultiWidgetEditor>();

    if (m_StdMultiWidget == stdMultiWidgetEditor->GetStdMultiWidget())
    {
      m_StdMultiWidget->RemovePlanesFromDataStorage();
    }
  }
}

void QmitkStdMultiWidgetEditor::PartVisible( berry::IWorkbenchPartReference::Pointer partRef )
{
  if (partRef->GetId() == QmitkStdMultiWidgetEditor::EDITOR_ID)
  {
    QmitkStdMultiWidgetEditor::Pointer stdMultiWidgetEditor = partRef->GetPart(false).Cast<QmitkStdMultiWidgetEditor>();

    if (m_StdMultiWidget == stdMultiWidgetEditor->GetStdMultiWidget())
    {
      m_StdMultiWidget->AddPlanesToDataStorage();
    }
  }
}

void QmitkStdMultiWidgetEditor::PartHidden( berry::IWorkbenchPartReference::Pointer partRef )
{
  if (partRef->GetId() == QmitkStdMultiWidgetEditor::EDITOR_ID)
  {
    QmitkStdMultiWidgetEditor::Pointer stdMultiWidgetEditor = partRef->GetPart(false).Cast<QmitkStdMultiWidgetEditor>();

    if (m_StdMultiWidget == stdMultiWidgetEditor->GetStdMultiWidget())
    {
      m_StdMultiWidget->RemovePlanesFromDataStorage();
    }
  }
}

void QmitkStdMultiWidgetEditor::SetFocus()
{
  if (m_StdMultiWidget != 0)
    m_StdMultiWidget->setFocus();
}
