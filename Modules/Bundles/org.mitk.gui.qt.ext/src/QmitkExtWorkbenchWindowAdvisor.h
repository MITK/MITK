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

#include <cherryWorkbenchWindowAdvisor.h>

#include <cherryIPartListener.h>
#include <cherryIEditorPart.h>
#include <cherryIWorkbenchPage.h>
#include <cherryWorkbenchAdvisor.h>

#include "mitkQtCommonExtDll.h"

class MITK_QT_COMMON_EXT_EXPORT QmitkExtWorkbenchWindowAdvisor : public cherry::WorkbenchWindowAdvisor
{
public:

    QmitkExtWorkbenchWindowAdvisor(cherry::WorkbenchAdvisor* wbAdvisor,
        cherry::IWorkbenchWindowConfigurer::Pointer configurer);

    cherry::ActionBarAdvisor::Pointer CreateActionBarAdvisor(
        cherry::IActionBarConfigurer::Pointer configurer);

    void PostWindowCreate();

    void PreWindowOpen();

    void ShowViewToolbar(bool show);

private:

  /**
   * Hooks the listeners needed on the window
   *
   * @param configurer
   */
  void HookTitleUpdateListeners(cherry::IWorkbenchWindowConfigurer::Pointer configurer);

  std::string ComputeTitle();

  void RecomputeTitle();

  /**
   * Updates the window title. Format will be: [pageInput -]
   * [currentPerspective -] [editorInput -] [workspaceLocation -] productName
   * @param editorHidden TODO
   */
  void UpdateTitle(bool editorHidden);

  void PropertyChange(cherry::Object::Pointer source, int propId);

  cherry::IPartListener::Pointer titlePartListener;
  cherry::IPerspectiveListener::Pointer titlePerspectiveListener;
  cherry::IPropertyChangeListener::Pointer editorPropertyListener;
  friend struct cherry::PropertyChangeIntAdapter<QmitkExtWorkbenchWindowAdvisor>;
  friend class PartListenerForTitle;
  friend class PerspectiveListenerForTitle;

  cherry::IEditorPart::WeakPtr lastActiveEditor;
  cherry::IPerspectiveDescriptor::WeakPtr lastPerspective;
  cherry::IWorkbenchPage::WeakPtr lastActivePage;
  std::string lastEditorTitle;
  cherry::IAdaptable* lastInput;

  cherry::WorkbenchAdvisor* wbAdvisor;
  bool showViewToolbar;
};

#endif /*QMITKEXTWORKBENCHWINDOWADVISOR_H_*/
