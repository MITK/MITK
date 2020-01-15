/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkFunctionalityCoordinator_h
#define QmitkFunctionalityCoordinator_h

#include <berryIPartListener.h>
#include <berryIWindowListener.h>
#include <berryIWorkbenchWindow.h>

#include <set>
#include <org_mitk_gui_qt_common_legacy_Export.h>

class QmitkFunctionality;

///
/// \ingroup org_mitk_gui_qt_common_legacy
///
/// A class which coordinates active QmitkFunctionalities, e.g. calling activated and hidden on them.
///
class MITK_QT_COMMON_LEGACY QmitkFunctionalityCoordinator : virtual public berry::IPartListener, virtual public berry::IWindowListener
{
public:

  QmitkFunctionalityCoordinator();

  ~QmitkFunctionalityCoordinator() override;

  ///
  /// Add listener
  ///
  void Start();

  ///
  /// Remove listener
  ///
  void Stop();

  //#IPartListener methods (these methods internally call Activated() or other similar methods)
  ///
  /// \see IPartListener::GetPartEventTypes()
  ///
  berry::IPartListener::Events::Types GetPartEventTypes() const override;
  ///
  /// \see IPartListener::PartActivated()
  ///
  void PartActivated (const berry::IWorkbenchPartReference::Pointer& partRef) override;
  ///
  /// \see IPartListener::PartDeactivated()
  ///
  void PartDeactivated(const berry::IWorkbenchPartReference::Pointer& /*partRef*/) override;
  ///
  /// \see IPartListener::PartOpened()
  ///
  void PartOpened(const berry::IWorkbenchPartReference::Pointer& partRef) override;
  ///
  /// \see IPartListener::PartClosed()
  ///
  void PartClosed (const berry::IWorkbenchPartReference::Pointer& partRef) override;
  ///
  /// \see IPartListener::PartHidden()
  ///
  void PartHidden (const berry::IWorkbenchPartReference::Pointer& partRef) override;
  ///
  /// \see IPartListener::PartVisible()
  ///
  void PartVisible (const berry::IWorkbenchPartReference::Pointer& partRef) override;

  /**
  * Notifies this listener that the given window has been closed.
  */
  void WindowClosed(const berry::IWorkbenchWindow::Pointer& window) override;

  /**
  * Notifies this listener that the given window has been opened.
  */
  void WindowOpened(const berry::IWorkbenchWindow::Pointer& /*window*/) override;

protected:
  ///
  /// Activates the standalone functionality
  ///
  void ActivateStandaloneFunctionality(berry::IWorkbenchPartReference *partRef);
  ///
  /// Deactivates the standalone functionality
  ///
  void DeactivateStandaloneFunctionality(berry::IWorkbenchPartReference *functionality, berry::IWorkbenchPartReference *newRef);
  ///
  /// Saves the workbench window
  ///
  berry::IWorkbenchWindow::WeakPtr m_Window;
  ///
  /// Saves the last part that added interactors
  ///
  berry::IWorkbenchPartReference* m_StandaloneFuntionality;

  ///
  /// Saves all opened QmitkFclassunctionalities
  ///
  std::set<QmitkFunctionality*> m_Functionalities;
  ///
  /// Saves all visible QmitkFunctionalities
  ///
  std::set<berry::IWorkbenchPartReference*> m_VisibleStandaloneFunctionalities;
};

#endif // QmitkFunctionalityCoordinator_h
