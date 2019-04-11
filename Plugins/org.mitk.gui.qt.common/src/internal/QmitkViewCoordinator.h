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


#ifndef QmitkViewCoordinator_h
#define QmitkViewCoordinator_h

#include <berryIPartListener.h>
#include <berryIWindowListener.h>
#include <berryIWorkbenchWindow.h>

#include <mitkILifecycleAwarePart.h>

#include <QSet>

namespace mitk {

struct IRenderWindowPart;
struct IRenderWindowPartListener;
struct IZombieViewPart;

}

class QmitkAbstractView;

/**
 * A class which coordinates active QmitkAbstractView s, e.g. calling activated and hidden on them.
 */
class QmitkViewCoordinator : private berry::IPartListener, private berry::IWindowListener
{
public:

  /**
   * Add listener
   */
  QmitkViewCoordinator();

  /**
   * Remove listener
   */
  ~QmitkViewCoordinator() override;

  void Start();
  void Stop();


  //#IPartListener methods (these methods internally call Activated() or other similar methods)

  /**
   * \see IPartListener::GetPartEventTypes()
   */
  berry::IPartListener::Events::Types GetPartEventTypes() const override;

  /**
   * \see IPartListener::PartActivated()
   */
  void PartActivated (const berry::IWorkbenchPartReference::Pointer& partRef) override;

  /**
   * \see IPartListener::PartDeactivated()
   */
  void PartDeactivated(const berry::IWorkbenchPartReference::Pointer& /*partRef*/) override;

  /**
   * \see IPartListener::PartOpened()
   */
  void PartOpened(const berry::IWorkbenchPartReference::Pointer& partRef) override;

  /**
   * \see IPartListener::PartClosed()
   */
  void PartClosed (const berry::IWorkbenchPartReference::Pointer& partRef) override;

  /**
   * \see IPartListener::PartHidden()
   */
  void PartHidden(const berry::IWorkbenchPartReference::Pointer& partRef) override;

  /**
   * \see IPartListener::PartVisible()
   */
  void PartVisible(const berry::IWorkbenchPartReference::Pointer& partRef) override;

  /**
  * \see IPartListener::PartInputChanged()
  */
  void PartInputChanged(const berry::IWorkbenchPartReference::Pointer& partRef) override;

  /**
  * Notifies this listener that the given window has been opened.
  */
  void WindowOpened(const berry::IWorkbenchWindow::Pointer& window) override;

  /**
   * Notifies this listener that the given window has been closed.
   */
  void WindowClosed(const berry::IWorkbenchWindow::Pointer& window) override;

private:

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderPart);
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderPart);
  void RenderWindowPartInputChanged(mitk::IRenderWindowPart* renderPart);

private:

  mitk::IZombieViewPart* m_ActiveZombieView;
  mitk::IRenderWindowPart* m_ActiveRenderWindowPart;
  mitk::IRenderWindowPart* m_VisibleRenderWindowPart;

  QSet<mitk::IRenderWindowPartListener*> m_RenderWindowListeners;

};

#endif // QmitkViewCoordinator_h
