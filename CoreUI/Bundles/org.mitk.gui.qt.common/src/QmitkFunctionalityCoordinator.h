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
#include "mitkQtCommonDll.h"
class QmitkFunctionality;

///
/// A class which coordinates active QmitkFunctionalities, e.g. calling activated and hidden on them.
///
class MITK_QT_COMMON QmitkFunctionalityCoordinator : virtual public berry::IPartListener, virtual public berry::IWindowListener
{
public:
  berryObjectMacro(QmitkFunctionalityCoordinator);
  berryNewMacro(QmitkFunctionalityCoordinator);

  ///
  /// Add listener
  ///
  QmitkFunctionalityCoordinator();
  ///
  /// Remove listener
  ///
  virtual ~QmitkFunctionalityCoordinator();
  ///
  /// Sets the window to attach the part listener
  ///
  void SetWindow(berry::IWorkbenchWindow::Pointer window);
  //#IPartListener methods (these methods internally call Activated() or other similar methods)
  ///
  /// \see IPartListener::GetPartEventTypes()
  ///
  Events::Types GetPartEventTypes() const;
  ///
  /// \see IPartListener::PartActivated()
  ///
  virtual void PartActivated (berry::IWorkbenchPartReference::Pointer partRef);
  ///
  /// \see IPartListener::PartDeactivated()
  ///
  virtual void PartDeactivated(berry::IWorkbenchPartReference::Pointer partRef);
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
  virtual void WindowOpened(berry::IWorkbenchWindow::Pointer window);
protected:
  ///
  /// Activates the standalone functionality
  ///
  void ActivateStandaloneFunctionality(QmitkFunctionality* functionality);
  ///
  /// Deactivates the standalone functionality
  ///
  void DeactivateStandaloneFunctionality(QmitkFunctionality* functionality);
  ///
  /// Saves the workbench window
  ///
  berry::IWorkbenchWindow::WeakPtr m_Window;
  ///
  /// Saves the last QmitkFunctionality that added interactors
  ///
  QmitkFunctionality* m_StandaloneFuntionality;
  ///
  /// Saves all opened QmitkFunctionalities
  ///
  std::set<QmitkFunctionality*> m_Functionalities;
  ///
  /// Saves all visible QmitkFunctionalities
  ///
  std::set<QmitkFunctionality*> m_VisibleStandaloneFunctionalities;
};

#endif // QmitkFunctionalityCoordinator_h
