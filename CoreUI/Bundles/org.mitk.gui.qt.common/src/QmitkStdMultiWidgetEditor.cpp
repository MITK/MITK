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

#include <cherryUIException.h>
#include <cherryIWorkbenchPage.h>
#include <cherryIPreferencesService.h>

#include <QWidget>

#include <mitkGlobalInteraction.h>

#include <mitkDataStorageEditorInput.h>

const std::string QmitkStdMultiWidgetEditor::EDITOR_ID = "org.mitk.editors.stdmultiwidget";

QmitkStdMultiWidgetEditor::QmitkStdMultiWidgetEditor()
 : m_StdMultiWidget(0)
{

}

QmitkStdMultiWidgetEditor::~QmitkStdMultiWidgetEditor()
{
  // we need to wrap the RemovePartListener call inside a
  // register/unregister block to prevent infinite recursion
  // due to the destruction of temporary smartpointer to this
  this->Register();
  this->GetSite()->GetPage()->RemovePartListener(cherry::IPartListener::Pointer(this));
  this->UnRegister(false);
}

QmitkStdMultiWidget* QmitkStdMultiWidgetEditor::GetStdMultiWidget()
{
  return m_StdMultiWidget;
}

void QmitkStdMultiWidgetEditor::Init(cherry::IEditorSite::Pointer site, cherry::IEditorInput::Pointer input)
{
  if (input.Cast<mitk::DataStorageEditorInput>().IsNull())
     throw cherry::PartInitException("Invalid Input: Must be IFileEditorInput");

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
    this->GetSite()->GetPage()->AddPartListener(cherry::IPartListener::Pointer(this));

    // enable change of logo
    cherry::IPreferencesService::Pointer prefService
      = cherry::Platform::GetServiceRegistry()
      .GetServiceById<cherry::IPreferencesService>(cherry::IPreferencesService::ID);

    cherry::IPreferences::Pointer logoPref = prefService->GetSystemPreferences()->Node("DepartmentLogo");
    std::string departmentLogoLocation = logoPref->Get("DepartmentLogo","");

    m_StdMultiWidget->SetDepartmentLogoPath(departmentLogoLocation.c_str());
    m_StdMultiWidget->DisableDepartmentLogo();
    m_StdMultiWidget->EnableDepartmentLogo();
  }
}

cherry::IPartListener::Events::Types QmitkStdMultiWidgetEditor::GetPartEventTypes() const
{
  return Events::CLOSED | Events::HIDDEN | Events::VISIBLE;
}

void QmitkStdMultiWidgetEditor::PartClosed( cherry::IWorkbenchPartReference::Pointer partRef )
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

void QmitkStdMultiWidgetEditor::PartVisible( cherry::IWorkbenchPartReference::Pointer partRef )
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

void QmitkStdMultiWidgetEditor::PartHidden( cherry::IWorkbenchPartReference::Pointer partRef )
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
