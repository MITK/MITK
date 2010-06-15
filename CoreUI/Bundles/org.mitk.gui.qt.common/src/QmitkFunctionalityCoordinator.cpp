/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkFunctionalityCoordinator.h"
#include "QmitkFunctionality.h"
#include <QmitkStdMultiWidgetEditor.h>
#include <berryPlatformUI.h>
#include <berryIWorkbenchPage.h>

QmitkFunctionalityCoordinator::QmitkFunctionalityCoordinator()
: m_StandaloneFuntionality(NULL)
{
}

void QmitkFunctionalityCoordinator::SetWindow( berry::IWorkbenchWindow::Pointer window )
{
  m_Window = window;
  if(window.IsNotNull())
  {
    window->GetWorkbench()->AddWindowListener(berry::IWindowListener::Pointer(this));
    window->GetPartService()->AddPartListener(berry::IPartListener::Pointer(this));
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
  // Check for QmitkFunctionality
  QmitkFunctionality::Pointer _QmitkFunctionality = partRef->GetPart(false).Cast<QmitkFunctionality>();
  // change the active standalone functionality
  this->ActivateStandaloneFunctionality(_QmitkFunctionality.GetPointer());
}

void QmitkFunctionalityCoordinator::PartDeactivated( berry::IWorkbenchPartReference::Pointer partRef )
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
      this->DeactivateStandaloneFunctionality(_QmitkFunctionality.GetPointer());
      
      // and set pointer to 0
      if(m_StandaloneFuntionality == _QmitkFunctionality.GetPointer())
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
    // try to deactivate on hide (if is activated)
    this->DeactivateStandaloneFunctionality(_QmitkFunctionality.GetPointer());

    // tracking of Visible Standalone Functionalities
    m_VisibleStandaloneFunctionalities.erase(_QmitkFunctionality.GetPointer());
    // activate Functionality if just one Standalone Functionality is visible (old one gets deactivated)
    if(m_VisibleStandaloneFunctionalities.size() == 1)
      this->ActivateStandaloneFunctionality(_QmitkFunctionality.GetPointer());
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
    m_VisibleStandaloneFunctionalities.insert(_QmitkFunctionality.GetPointer());
    // activate Functionality if just one Standalone Functionality is visible
    if(m_VisibleStandaloneFunctionalities.size() == 1)
      this->ActivateStandaloneFunctionality(_QmitkFunctionality.GetPointer());
  }
}

void QmitkFunctionalityCoordinator::ActivateStandaloneFunctionality( QmitkFunctionality* functionality ) 
{
  if(functionality && !functionality->IsActivated() && functionality->IsExclusiveFunctionality())
  {
    // deactivate old one if necessary
    this->DeactivateStandaloneFunctionality(m_StandaloneFuntionality);
    m_StandaloneFuntionality = functionality;

    // call activated on this functionality
    m_StandaloneFuntionality->SetActivated(true);
    m_StandaloneFuntionality->Activated();  
  }
}

void QmitkFunctionalityCoordinator::DeactivateStandaloneFunctionality(QmitkFunctionality* functionality)
{
  if(functionality && functionality->IsActivated())
  {
    functionality->SetActivated(false);
    functionality->Deactivated();
  }
}

void QmitkFunctionalityCoordinator::WindowClosed( berry::IWorkbenchWindow::Pointer window )
{
  if(window.IsNotNull())
  {
    window->GetWorkbench()->RemoveWindowListener(berry::IWindowListener::Pointer(this));
    window->GetPartService()->RemovePartListener(berry::IPartListener::Pointer(this));
  }
}

void QmitkFunctionalityCoordinator::WindowOpened( berry::IWorkbenchWindow::Pointer window )
{

}