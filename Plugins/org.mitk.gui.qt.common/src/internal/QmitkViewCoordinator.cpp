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

#include "QmitkViewCoordinator.h"
#include "QmitkAbstractView.h"

#include <mitkIRenderWindowPart.h>
#include <mitkIRenderWindowPartListener.h>
#include <mitkIZombieViewPart.h>

#include <berryPlatformUI.h>
#include <berryIWorkbenchPage.h>

QmitkViewCoordinator::QmitkViewCoordinator()
  : m_ActiveZombieView(0)
  , m_ActiveRenderWindowPart(0)
  , m_VisibleRenderWindowPart(0)
{
}

QmitkViewCoordinator::~QmitkViewCoordinator()
{
}

void QmitkViewCoordinator::Start()
{
  berry::PlatformUI::GetWorkbench()->AddWindowListener(berry::IWindowListener::Pointer(this));
  std::vector<berry::IWorkbenchWindow::Pointer> wnds(berry::PlatformUI::GetWorkbench()->GetWorkbenchWindows());
  for (std::vector<berry::IWorkbenchWindow::Pointer>::iterator i = wnds.begin();
       i != wnds.end(); ++i)
  {
    (*i)->GetPartService()->AddPartListener(berry::IPartListener::Pointer(this));
  }
}

void QmitkViewCoordinator::Stop()
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

berry::IPartListener::Events::Types QmitkViewCoordinator::GetPartEventTypes() const
{
  return berry::IPartListener::Events::ACTIVATED | berry::IPartListener::Events::DEACTIVATED
    | berry::IPartListener::Events::CLOSED | berry::IPartListener::Events::HIDDEN
    | berry::IPartListener::Events::VISIBLE | berry::IPartListener::Events::OPENED;
}

void QmitkViewCoordinator::PartActivated( berry::IWorkbenchPartReference::Pointer partRef )
{
  //MITK_INFO << "*** PartActivated (" << partRef->GetPart(false)->GetPartName() << ")";
  berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

  // Check for a render window part and inform IRenderWindowPartListener views
  // that it was activated
  if ( mitk::IRenderWindowPart* renderPart = dynamic_cast<mitk::IRenderWindowPart*>(part) )
  {
    if (m_VisibleRenderWindowPart != renderPart)
    {
      RenderWindowPartActivated(renderPart);
      m_ActiveRenderWindowPart = renderPart;
      m_VisibleRenderWindowPart = renderPart;
    }
  }

  // Check if the activated part wants to be notified
  if (mitk::ILifecycleAwarePart* lifecycleAwarePart = dynamic_cast<mitk::ILifecycleAwarePart*>(part))
  {
    lifecycleAwarePart->Activated();
  }

  // Check if a zombie view has been activated.
  if (mitk::IZombieViewPart* zombieView = dynamic_cast<mitk::IZombieViewPart*>(part))
  {
    if (m_ActiveZombieView && (m_ActiveZombieView != zombieView))
    {
      // Another zombie view has been activated. Tell the old one about it.
      m_ActiveZombieView->ActivatedZombieView(partRef);
      m_ActiveZombieView = zombieView;
    }
  }
}

void QmitkViewCoordinator::PartDeactivated( berry::IWorkbenchPartReference::Pointer partRef )
{
  //MITK_INFO << "*** PartDeactivated (" << partRef->GetPart(false)->GetPartName() << ")";
  berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

  if (mitk::ILifecycleAwarePart* lifecycleAwarePart = dynamic_cast<mitk::ILifecycleAwarePart*>(part))
  {
    lifecycleAwarePart->Deactivated();
  }
}

void QmitkViewCoordinator::PartOpened( berry::IWorkbenchPartReference::Pointer partRef )
{
  //MITK_INFO << "*** PartOpened (" << partRef->GetPart(false)->GetPartName() << ")";
  berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

  if (mitk::IRenderWindowPartListener* renderWindowListener = dynamic_cast<mitk::IRenderWindowPartListener*>(part))
  {
    m_RenderWindowListeners.insert(renderWindowListener);
  }
}

void QmitkViewCoordinator::PartClosed( berry::IWorkbenchPartReference::Pointer partRef )
{
  //MITK_INFO << "*** PartClosed (" << partRef->GetPart(false)->GetPartName() << ")";
  berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

  if (mitk::IRenderWindowPartListener* renderWindowListener = dynamic_cast<mitk::IRenderWindowPartListener*>(part))
  {
    m_RenderWindowListeners.remove(renderWindowListener);
  }
}

void QmitkViewCoordinator::PartHidden( berry::IWorkbenchPartReference::Pointer partRef )
{
  //MITK_INFO << "*** PartHidden (" << partRef->GetPart(false)->GetPartName() << ")";
  berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

  // Check for a render window part and if it is the currently active on.
  // Inform IRenderWindowPartListener views that it has been hidden.
  if ( mitk::IRenderWindowPart* renderPart = dynamic_cast<mitk::IRenderWindowPart*>(part) )
  {
    if (!m_ActiveRenderWindowPart && m_VisibleRenderWindowPart == renderPart)
    {
      RenderWindowPartDeactivated(renderPart);
      m_VisibleRenderWindowPart = 0;
    }
  }

  if (mitk::ILifecycleAwarePart* lifecycleAwarePart = dynamic_cast<mitk::ILifecycleAwarePart*>(part))
  {
    lifecycleAwarePart->Hidden();
  }
}

void QmitkViewCoordinator::PartVisible( berry::IWorkbenchPartReference::Pointer partRef )
{
  //MITK_INFO << "*** PartVisible (" << partRef->GetPart(false)->GetPartName() << ")";
  berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

  // Check for a render window part and inform IRenderWindowPartListener views
  // that it was activated
  if ( mitk::IRenderWindowPart* renderPart = dynamic_cast<mitk::IRenderWindowPart*>(part) )
  {
    if (!m_ActiveRenderWindowPart)
    {
      RenderWindowPartActivated(renderPart);
      m_VisibleRenderWindowPart = renderPart;
    }
  }

  if (mitk::ILifecycleAwarePart* lifecycleAwarePart = dynamic_cast<mitk::ILifecycleAwarePart*>(part))
  {
    lifecycleAwarePart->Visible();
  }
}

void QmitkViewCoordinator::RenderWindowPartActivated(mitk::IRenderWindowPart* renderPart)
{
  foreach (mitk::IRenderWindowPartListener* l, m_RenderWindowListeners)
  {
    l->RenderWindowPartActivated(renderPart);
  }
}

void QmitkViewCoordinator::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderPart)
{
  foreach (mitk::IRenderWindowPartListener* l, m_RenderWindowListeners)
  {
    l->RenderWindowPartDeactivated(renderPart);
  }
}

void QmitkViewCoordinator::WindowClosed( berry::IWorkbenchWindow::Pointer /*window*/ )
{
}

void QmitkViewCoordinator::WindowOpened( berry::IWorkbenchWindow::Pointer window )
{
  window->GetPartService()->AddPartListener(berry::IPartListener::Pointer(this));
}

