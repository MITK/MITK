/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITKEXTWORKBENCHWINDOWADVISOR_H_
#define QMITKEXTWORKBENCHWINDOWADVISOR_H_

#include <berryWorkbenchWindowAdvisor.h>

#include <berryIPartListener.h>
#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>
#include <berryWorkbenchAdvisor.h>
#include "mitkQtCommonExtDll.h"
#include "QmitkCommonWorkbenchWindowAdvisor.h"

#include <QList>
class QAction;
class QMenu;

class MITK_QT_COMMON_EXT_EXPORT QmitkExtWorkbenchWindowAdvisor : public QmitkCommonWorkbenchWindowAdvisor
{
public:

    QmitkExtWorkbenchWindowAdvisor(berry::WorkbenchAdvisor* wbAdvisor,
        berry::IWorkbenchWindowConfigurer::Pointer configurer);

    berry::ActionBarAdvisor::Pointer CreateActionBarAdvisor(
        berry::IActionBarConfigurer::Pointer configurer);

    void* CreateEmptyWindowContents(void* parent);

    void PostWindowCreate();

    void PreWindowOpen();

    void ShowViewToolbar(bool show);

    void ShowVersionInfo(bool show);

    //TODO should be removed when product support is here
    void SetProductName(const std::string& product);
    void SetWindowIcon(const std::string& wndIcon);

private:

  /**
   * Hooks the listeners needed on the window
   *
   * @param configurer
   */
  void HookTitleUpdateListeners(berry::IWorkbenchWindowConfigurer::Pointer configurer);

  std::string ComputeTitle();

  void RecomputeTitle();

  /**
   * Updates the window title. Format will be: [pageInput -]
   * [currentPerspective -] [editorInput -] [workspaceLocation -] productName
   * @param editorHidden TODO
   */
  void UpdateTitle(bool editorHidden);

  void PropertyChange(berry::Object::Pointer source, int propId);

  berry::IPartListener::Pointer titlePartListener;
  berry::IPerspectiveListener::Pointer titlePerspectiveListener;
  berry::IPartListener::Pointer imageNavigatorPartListener;
  berry::IPropertyChangeListener::Pointer editorPropertyListener;
  friend struct berry::PropertyChangeIntAdapter<QmitkExtWorkbenchWindowAdvisor>;
  friend class PartListenerForTitle;
  friend class PerspectiveListenerForTitle;
  friend class PartListenerForImageNavigator;

  berry::IEditorPart::WeakPtr lastActiveEditor;
  berry::IPerspectiveDescriptor::WeakPtr lastPerspective;
  berry::IWorkbenchPage::WeakPtr lastActivePage;
  std::string lastEditorTitle;
  berry::IAdaptable* lastInput;

  berry::WorkbenchAdvisor* wbAdvisor;
  bool showViewToolbar;
  bool showVersionInfo;
  std::string productName;
  std::string windowIcon;

  // actions which will be enabled/disabled depending on the application state
  QList<QAction*> viewActions;
  QAction* fileSaveProjectAction;
  QAction* closeProjectAction;
  QAction* undoAction;
  QAction* redoAction;
  QAction* imageNavigatorAction;
  QAction* resetPerspAction;
  QAction* closePerspAction;
};

#endif /*QMITKEXTWORKBENCHWINDOWADVISOR_H_*/
