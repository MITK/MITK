/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkViewCoordinator.h"
#include "QmitkAbstractView.h"

#include <mitkIRenderWindowPart.h>
#include <mitkIRenderWindowPartListener.h>
#include <mitkIZombieViewPart.h>

#include <berryPlatformUI.h>
#include <berryIWorkbenchPage.h>

QmitkViewCoordinator::QmitkViewCoordinator()
  : m_ActiveZombieView(nullptr)
  , m_CurrentRenderWindowPart(nullptr)
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
  berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

  // Render-window listener dispatch is driven by PartVisible/PartClosed, not by
  // focus changes: a focus-driven deactivation should not disable views that
  // depend on a render window part. See PartVisible/PartClosed below.

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
  berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

  if (mitk::ILifecycleAwarePart* lifecycleAwarePart = dynamic_cast<mitk::ILifecycleAwarePart*>(part))
  {
    lifecycleAwarePart->Deactivated();
  }
}

void QmitkViewCoordinator::PartOpened(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

  if (mitk::IRenderWindowPartListener* renderWindowListener = dynamic_cast<mitk::IRenderWindowPartListener*>(part))
  {
    m_RenderWindowListeners.insert(renderWindowListener);
  }
}

void QmitkViewCoordinator::PartClosed(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

  // If the closing part is the render window part we last notified listeners
  // about, notify them that it is gone.
  if (mitk::IRenderWindowPart* renderPart = dynamic_cast<mitk::IRenderWindowPart*>(part))
  {
    if (m_CurrentRenderWindowPart == renderPart)
    {
      RenderWindowPartDeactivated(renderPart);
      m_CurrentRenderWindowPart = nullptr;
    }
  }

  if (mitk::IRenderWindowPartListener* renderWindowListener = dynamic_cast<mitk::IRenderWindowPartListener*>(part))
  {
    m_RenderWindowListeners.remove(renderWindowListener);
  }
}

void QmitkViewCoordinator::PartHidden(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

  // Do not dispatch RenderWindowPartDeactivated on transient hide. During
  // startup the Welcome page can cover the render window editor, which
  // would otherwise spuriously disable listener views. PartClosed is the
  // authoritative signal that a render window part is gone.

  if (mitk::ILifecycleAwarePart* lifecycleAwarePart = dynamic_cast<mitk::ILifecycleAwarePart*>(part))
  {
    lifecycleAwarePart->Hidden();
  }
}

void QmitkViewCoordinator::PartVisible(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

  if (mitk::IRenderWindowPart* renderPart = dynamic_cast<mitk::IRenderWindowPart*>(part))
  {
    if (m_CurrentRenderWindowPart != renderPart)
    {
      if (nullptr != m_CurrentRenderWindowPart)
      {
        RenderWindowPartDeactivated(m_CurrentRenderWindowPart);
      }
      RenderWindowPartActivated(renderPart);
      m_CurrentRenderWindowPart = renderPart;
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

  if (mitk::IRenderWindowPart* renderPart = dynamic_cast<mitk::IRenderWindowPart*>(part))
  {
    if (m_CurrentRenderWindowPart == renderPart)
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
  for (auto& listener : std::as_const(m_RenderWindowListeners))
  {
    listener->RenderWindowPartActivated(renderPart);
  }
}

void QmitkViewCoordinator::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderPart)
{
  for (auto& listener : std::as_const(m_RenderWindowListeners))
  {
    listener->RenderWindowPartDeactivated(renderPart);
  }
}

void QmitkViewCoordinator::RenderWindowPartInputChanged(mitk::IRenderWindowPart* renderPart)
{
  for (auto& listener : std::as_const(m_RenderWindowListeners))
  {
    listener->RenderWindowPartInputChanged(renderPart);
  }
}
