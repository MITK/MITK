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

#include "QmitkFunctionalityCoordinator.h"
#include "QmitkFunctionality.h"
#include <QmitkStdMultiWidgetEditor.h>
#include <berryPlatformUI.h>
#include <berryIWorkbenchPage.h>

#include <mitkIZombieViewPart.h>

QmitkFunctionalityCoordinator::QmitkFunctionalityCoordinator()
  : m_StandaloneFuntionality(0)
{
}

void QmitkFunctionalityCoordinator::Start()
{
  berry::PlatformUI::GetWorkbench()->AddWindowListener(berry::IWindowListener::Pointer(this));
  std::vector<berry::IWorkbenchWindow::Pointer> wnds(berry::PlatformUI::GetWorkbench()->GetWorkbenchWindows());
  for (std::vector<berry::IWorkbenchWindow::Pointer>::iterator i = wnds.begin();
       i != wnds.end(); ++i)
  {
    (*i)->GetPartService()->AddPartListener(berry::IPartListener::Pointer(this));
  }
}

void QmitkFunctionalityCoordinator::Stop()
{
  if (!berry::PlatformUI::IsWorkbenchRunning()) return;

  berry::PlatformUI::GetWorkbench()->RemoveWindowListener(berry::IWindowListener::Pointer(this));
  std::vector<berry::IWorkbenchWindow::Pointer> wnds(berry::PlatformUI::GetWorkbench()->GetWorkbenchWindows());
  for (std::vector<berry::IWorkbenchWindow::Pointer>::iterator i = wnds.begin();
       i != wnds.end(); ++i)
  {
    (*i)->GetPartService()->RemovePartListener(berry::IPartListener::Pointer(this));
  }
}

QmitkFunctionalityCoordinator::~QmitkFunctionalityCoordinator()
{
}

berry::IPartListener::Events::Types QmitkFunctionalityCoordinator::GetPartEventTypes() const
{
  return berry::IPartListener::Events::ACTIVATED | berry::IPartListener::Events::DEACTIVATED
    | berry::IPartListener::Events::CLOSED | berry::IPartListener::Events::HIDDEN
    | berry::IPartListener::Events::VISIBLE | berry::IPartListener::Events::OPENED;
}

void QmitkFunctionalityCoordinator::PartActivated( berry::IWorkbenchPartReference::Pointer partRef )
{
  // change the active standalone functionality
  this->ActivateStandaloneFunctionality(partRef.GetPointer());
}

void QmitkFunctionalityCoordinator::PartDeactivated( berry::IWorkbenchPartReference::Pointer /*partRef*/ )
{
  // nothing to do here: see PartActivated()
}

void QmitkFunctionalityCoordinator::PartOpened( berry::IWorkbenchPartReference::Pointer partRef )
{
   // check for multiwidget and inform views that it is available now
  if ( partRef->GetId() == QmitkStdMultiWidgetEditor::EDITOR_ID )
  {
    for (std::set<QmitkFunctionality*>::iterator it = m_Functionalities.begin()
      ; it != m_Functionalities.end(); it++)
    {
      (*it)->StdMultiWidgetAvailable(*(partRef
        ->GetPart(false).Cast<QmitkStdMultiWidgetEditor>()->GetStdMultiWidget()));
    }
  }
  else
  {
    // Check for QmitkFunctionality
    QmitkFunctionality::Pointer _QmitkFunctionality = partRef->GetPart(false).Cast<QmitkFunctionality>();
    if(_QmitkFunctionality.IsNotNull())
    {
      m_Functionalities.insert(_QmitkFunctionality.GetPointer()); // save as opened functionality
    }
  }
}

void QmitkFunctionalityCoordinator::PartClosed( berry::IWorkbenchPartReference::Pointer partRef )
{
  // check for multiwidget and inform views that it not available any more
  if ( partRef->GetId() == QmitkStdMultiWidgetEditor::EDITOR_ID )
  {

    QmitkStdMultiWidgetEditor::Pointer stdMultiWidgetEditor = partRef->GetPart(false).Cast<QmitkStdMultiWidgetEditor>();
    for (std::set<QmitkFunctionality*>::iterator it = m_Functionalities.begin()
      ; it != m_Functionalities.end(); it++)
    {
      (*it)->StdMultiWidgetClosed(*(stdMultiWidgetEditor->GetStdMultiWidget()));
      (*it)->StdMultiWidgetNotAvailable(); // deprecated call, provided for consistence
    }
  }
  else
  {
    // check for functionality
    QmitkFunctionality::Pointer _QmitkFunctionality = partRef->GetPart(false).Cast<QmitkFunctionality>();
    if(_QmitkFunctionality.IsNotNull())
    {
      // deactivate on close ( the standalone functionality may still be activated  )
      this->DeactivateStandaloneFunctionality(partRef.GetPointer(), 0);

      // and set pointer to 0
      if(m_StandaloneFuntionality == partRef.GetPointer())
        m_StandaloneFuntionality = 0;

      m_Functionalities.erase(_QmitkFunctionality.GetPointer()); // remove as opened functionality

      // call PartClosed on the QmitkFunctionality
      _QmitkFunctionality->ClosePartProxy();
      //m_VisibleStandaloneFunctionalities.erase(_QmitkFunctionality.GetPointer()); // remove if necessary (should be done before in PartHidden()
    }
  }
}

void QmitkFunctionalityCoordinator::PartHidden( berry::IWorkbenchPartReference::Pointer partRef )
{
  // Check for QmitkFunctionality
  QmitkFunctionality::Pointer _QmitkFunctionality = partRef->GetPart(false).Cast<QmitkFunctionality>();
  if(_QmitkFunctionality != 0)
  {
    _QmitkFunctionality->SetVisible(false);
    _QmitkFunctionality->Hidden();

    // tracking of Visible Standalone Functionalities
    m_VisibleStandaloneFunctionalities.erase(partRef.GetPointer());

    // activate Functionality if just one Standalone Functionality is visible
    if( m_VisibleStandaloneFunctionalities.size() == 1 )
      this->ActivateStandaloneFunctionality( *m_VisibleStandaloneFunctionalities.begin() );
  }
}

void QmitkFunctionalityCoordinator::PartVisible( berry::IWorkbenchPartReference::Pointer partRef )
{
  // Check for QmitkFunctionality
  QmitkFunctionality::Pointer _QmitkFunctionality = partRef->GetPart(false).Cast<QmitkFunctionality>();
  if(_QmitkFunctionality.IsNotNull())
  {
    _QmitkFunctionality->SetVisible(true);
    _QmitkFunctionality->Visible();

    // tracking of Visible Standalone Functionalities
    if( _QmitkFunctionality->IsExclusiveFunctionality() )
    {
      m_VisibleStandaloneFunctionalities.insert(partRef.GetPointer());

      // activate Functionality if just one Standalone Functionality is visible
      if( m_VisibleStandaloneFunctionalities.size() == 1 )
        this->ActivateStandaloneFunctionality( *m_VisibleStandaloneFunctionalities.begin() );
    }
  }
}

void QmitkFunctionalityCoordinator::ActivateStandaloneFunctionality( berry::IWorkbenchPartReference* partRef )
{
  QmitkFunctionality* functionality = dynamic_cast<QmitkFunctionality*>(partRef->GetPart(false).GetPointer());
  if( functionality && !functionality->IsActivated() && functionality->IsExclusiveFunctionality() )
  {
    MITK_INFO << "**** Activating legacy standalone functionality";
    // deactivate old one if necessary
    this->DeactivateStandaloneFunctionality(m_StandaloneFuntionality, partRef);
    m_StandaloneFuntionality = partRef;

    MITK_INFO << "setting active flag";
    // call activated on this functionality
    functionality->SetActivated(true);
    functionality->Activated();
  }
  else if (dynamic_cast<mitk::IZombieViewPart*>(partRef->GetPart(false).GetPointer()) &&
           m_StandaloneFuntionality != partRef)
  {
    this->DeactivateStandaloneFunctionality(m_StandaloneFuntionality, partRef);
    m_StandaloneFuntionality = partRef;
  }
}

void QmitkFunctionalityCoordinator::DeactivateStandaloneFunctionality(berry::IWorkbenchPartReference* partRef,
                                                                      berry::IWorkbenchPartReference* newRef)
{
  if (partRef == 0) return;

  QmitkFunctionality* functionality = dynamic_cast<QmitkFunctionality*>(partRef->GetPart(false).GetPointer());
  if(functionality && functionality->IsActivated())
  {
    functionality->SetActivated(false);
    functionality->Deactivated();
  }
  else if (mitk::IZombieViewPart* zombie = dynamic_cast<mitk::IZombieViewPart*>(partRef->GetPart(false).GetPointer()))
  {
    zombie->ActivatedZombieView(berry::IWorkbenchPartReference::Pointer(newRef));
  }
}

void QmitkFunctionalityCoordinator::WindowClosed( berry::IWorkbenchWindow::Pointer /*window*/ )
{

}

void QmitkFunctionalityCoordinator::WindowOpened( berry::IWorkbenchWindow::Pointer window )
{
  window->GetPartService()->AddPartListener(berry::IPartListener::Pointer(this));
}
