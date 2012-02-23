/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-01-16 19:57:43 +0100 (Sa, 16 Jan 2010) $
Version:   $Revision: 21070 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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
  berryObjectMacro(QmitkFunctionalityCoordinator);
  berryNewMacro(QmitkFunctionalityCoordinator);

  QmitkFunctionalityCoordinator();

  virtual ~QmitkFunctionalityCoordinator();

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
  berry::IPartListener::Events::Types GetPartEventTypes() const;
  ///
  /// \see IPartListener::PartActivated()
  ///
  virtual void PartActivated (berry::IWorkbenchPartReference::Pointer partRef);
  ///
  /// \see IPartListener::PartDeactivated()
  ///
  virtual void PartDeactivated(berry::IWorkbenchPartReference::Pointer /*partRef*/);
  ///
  /// \see IPartListener::PartOpened()
  ///
  virtual void PartOpened(berry::IWorkbenchPartReference::Pointer partRef);
  ///
  /// \see IPartListener::PartClosed()
  ///
  virtual void PartClosed (berry::IWorkbenchPartReference::Pointer partRef);
  ///
  /// \see IPartListener::PartHidden()
  ///
  virtual void PartHidden (berry::IWorkbenchPartReference::Pointer partRef);
  ///
  /// \see IPartListener::PartVisible()
  ///
  virtual void PartVisible (berry::IWorkbenchPartReference::Pointer partRef);

  /**
  * Notifies this listener that the given window has been closed.
  */
  virtual void WindowClosed(berry::IWorkbenchWindow::Pointer window);

  /**
  * Notifies this listener that the given window has been opened.
  */
  virtual void WindowOpened(berry::IWorkbenchWindow::Pointer /*window*/);

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
