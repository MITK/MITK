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
  : m_ActiveZombieView(nullptr)
  , m_ActiveRenderWindowPart(nullptr)
  , m_VisibleRenderWindowPart(nullptr)
{
}

QmitkViewCoordinator::~QmitkViewCoordinator()
{
}

void QmitkViewCoordinator::Start()
{
  berry::PlatformUI::GetWorkbench()->AddWindowListener(this);
  QList<berry::IWorkbenchWindow::Pointer> wnds(berry::PlatformUI::GetWorkbench()->GetWorkbenchWindows());
  for (QList<berry::IWorkbenchWindow::Pointer>::iterator i = wnds.begin(); i != wnds.end(); ++i)
  {
    (*i)->GetPartService()->AddPartListener(this);
  }
}

void QmitkViewCoordinator::Stop()
{
  if (!berry::PlatformUI::IsWorkbenchRunning()) return;

  berry::PlatformUI::GetWorkbench()->RemoveWindowListener(this);
  QList<berry::IWorkbenchWindow::Pointer> wnds(berry::PlatformUI::GetWorkbench()->GetWorkbenchWindows());
  for (QList<berry::IWorkbenchWindow::Pointer>::iterator i = wnds.begin(); i != wnds.end(); ++i)
  {
    (*i)->GetPartService()->RemovePartListener(this);
  }
}

berry::IPartListener::Events::Types QmitkViewCoordinator::GetPartEventTypes() const
{
  return berry::IPartListener::Events::ACTIVATED | berry::IPartListener::Events::DEACTIVATED
    | berry::IPartListener::Events::CLOSED | berry::IPartListener::Events::HIDDEN
    | berry::IPartListener::Events::VISIBLE | berry::IPartListener::Events::OPENED
    | berry::IPartListener::Events::INPUT_CHANGED;
}

void QmitkViewCoordinator::PartActivated(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  //MITK_INFO << "*** PartActivated (" << partRef->GetPart(false)->GetPartName() << ")";
  berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

  // Check for a render window part and inform IRenderWindowPartListener views
  // that it was activated
  if (mitk::IRenderWindowPart* renderPart = dynamic_cast<mitk::IRenderWindowPart*>(part))
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

void QmitkViewCoordinator::PartDeactivated(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  //MITK_INFO << "*** PartDeactivated (" << partRef->GetPart(false)->GetPartName() << ")";
  berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

  // Check for a render window part and inform IRenderWindowPartListener views
  // that it was deactivated
  if (mitk::IRenderWindowPart* renderPart = dynamic_cast<mitk::IRenderWindowPart*>(part))
  {
    if (m_ActiveRenderWindowPart == renderPart)
    {
      this->RenderWindowPartDeactivated(renderPart);
      m_ActiveRenderWindowPart = nullptr;
      m_VisibleRenderWindowPart = nullptr;
    }
  }

  if (mitk::ILifecycleAwarePart* lifecycleAwarePart = dynamic_cast<mitk::ILifecycleAwarePart*>(part))
  {
    lifecycleAwarePart->Deactivated();
  }
}

void QmitkViewCoordinator::PartOpened(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  //MITK_INFO << "*** PartOpened (" << partRef->GetPart(false)->GetPartName() << ")";
  berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

  if (mitk::IRenderWindowPartListener* renderWindowListener = dynamic_cast<mitk::IRenderWindowPartListener*>(part))
  {
    m_RenderWindowListeners.insert(renderWindowListener);
  }
}

void QmitkViewCoordinator::PartClosed(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  //MITK_INFO << "*** PartClosed (" << partRef->GetPart(false)->GetPartName() << ")";
  berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

  if (mitk::IRenderWindowPartListener* renderWindowListener = dynamic_cast<mitk::IRenderWindowPartListener*>(part))
  {
    m_RenderWindowListeners.remove(renderWindowListener);
  }
}

void QmitkViewCoordinator::PartHidden(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  //MITK_INFO << "*** PartHidden (" << partRef->GetPart(false)->GetPartName() << ")";
  berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

  // Check for a render window part and if it is the currently active on.
  // Inform IRenderWindowPartListener views that it has been hidden.
  if (mitk::IRenderWindowPart* renderPart = dynamic_cast<mitk::IRenderWindowPart*>(part))
  {
    if (!m_ActiveRenderWindowPart && m_VisibleRenderWindowPart == renderPart)
    {
      RenderWindowPartDeactivated(renderPart);
      m_VisibleRenderWindowPart = nullptr;
    }
  }

  if (mitk::ILifecycleAwarePart* lifecycleAwarePart = dynamic_cast<mitk::ILifecycleAwarePart*>(part))
  {
    lifecycleAwarePart->Hidden();
  }
}

void QmitkViewCoordinator::PartVisible(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  //MITK_INFO << "*** PartVisible (" << partRef->GetPart(false)->GetPartName() << ")";
  berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

  // Check for a render window part and inform IRenderWindowPartListener views
  // that it was activated
  if (mitk::IRenderWindowPart* renderPart = dynamic_cast<mitk::IRenderWindowPart*>(part))
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

void QmitkViewCoordinator::PartInputChanged(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

  // Check for a render window part and inform IRenderWindowPartListener views
  // that it was changed
  if (mitk::IRenderWindowPart* renderPart = dynamic_cast<mitk::IRenderWindowPart*>(part))
  {
    if (!m_ActiveRenderWindowPart)
    {
      RenderWindowPartInputChanged(renderPart);
    }
  }
}

void QmitkViewCoordinator::WindowOpened(const berry::IWorkbenchWindow::Pointer& window)
{
  window->GetPartService()->AddPartListener(this);
}

void QmitkViewCoordinator::WindowClosed(const berry::IWorkbenchWindow::Pointer& /*window*/)
{
}

void QmitkViewCoordinator::RenderWindowPartActivated(mitk::IRenderWindowPart* renderPart)
{
  for (auto& listener : m_RenderWindowListeners)
  {
    listener->RenderWindowPartActivated(renderPart);
  }
}

void QmitkViewCoordinator::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderPart)
{
  for (auto& listener : m_RenderWindowListeners)
  {
    listener->RenderWindowPartDeactivated(renderPart);
  }
}

void QmitkViewCoordinator::RenderWindowPartInputChanged(mitk::IRenderWindowPart* renderPart)
{
  for (auto& listener : m_RenderWindowListeners)
  {
    listener->RenderWindowPartInputChanged(renderPart);
  }
}
