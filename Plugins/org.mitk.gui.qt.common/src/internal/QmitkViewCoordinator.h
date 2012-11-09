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
class QmitkViewCoordinator : public berry::IPartListener, public berry::IWindowListener
{
public:

  berryObjectMacro(QmitkViewCoordinator)

  /**
   * Add listener
   */
  QmitkViewCoordinator();

  /**
   * Remove listener
   */
  virtual ~QmitkViewCoordinator();

  void Start();
  void Stop();


  //#IPartListener methods (these methods internally call Activated() or other similar methods)

  /**
   * \see IPartListener::GetPartEventTypes()
   */
  berry::IPartListener::Events::Types GetPartEventTypes() const;

  /**
   * \see IPartListener::PartActivated()
   */
  virtual void PartActivated (berry::IWorkbenchPartReference::Pointer partRef);

  /**
   * \see IPartListener::PartDeactivated()
   */
  virtual void PartDeactivated(berry::IWorkbenchPartReference::Pointer /*partRef*/);

  /**
   * \see IPartListener::PartOpened()
   */
  virtual void PartOpened(berry::IWorkbenchPartReference::Pointer partRef);

  /**
   * \see IPartListener::PartClosed()
   */
  virtual void PartClosed (berry::IWorkbenchPartReference::Pointer partRef);

  /**
   * \see IPartListener::PartHidden()
   */
  virtual void PartHidden (berry::IWorkbenchPartReference::Pointer partRef);

  /**
   * \see IPartListener::PartVisible()
   */
  virtual void PartVisible (berry::IWorkbenchPartReference::Pointer partRef);

  /**
   * Notifies this listener that the given window has been closed.
   */
  virtual void WindowClosed(berry::IWorkbenchWindow::Pointer window);

  /**
   * Notifies this listener that the given window has been opened.
   */
  virtual void WindowOpened(berry::IWorkbenchWindow::Pointer /*window*/);

private:

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderPart);
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderPart);

private:

  mitk::IZombieViewPart* m_ActiveZombieView;
  mitk::IRenderWindowPart* m_ActiveRenderWindowPart;
  mitk::IRenderWindowPart* m_VisibleRenderWindowPart;

  QSet<mitk::IRenderWindowPartListener*> m_RenderWindowListeners;

};

#endif // QmitkViewCoordinator_h
