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

#ifndef CUSTOMVIEWERWORKBENCHWINDOWADVISOR_H_
#define CUSTOMVIEWERWORKBENCHWINDOWADVISOR_H_

#include <berryWorkbenchWindowAdvisor.h>
#include <QObject>
#include <QmitkFileOpenAction.h>

/**
 * \brief A WorkbenchWindowAdvisor class for the custom viewer plug-in.
 *
 * This class suits the custom viewer plug-in. Menu bar, tool bar and status bar are made invisible,
 * and the window title for the custom viewer is being set. The workbench window is being customized,
 * i.e. a perspectives tab-bar is arranged according to the PageComposite. The PageComposite is then
 * laid out according to perspective related contents by the WindowConfigurer.
 *
 * @see{ CustomViewerWorkbenchWindowAdvisor::PreWindowOpen(), CustomViewerWorkbenchWindowAdvisor::CreateWindowContents() }
 */
// //! [CustomViewerWorkbenchWindowAdvisorClassDeclaration]
class CustomViewerWorkbenchWindowAdvisor : public QObject, public berry::WorkbenchWindowAdvisor
// //! [CustomViewerWorkbenchWindowAdvisorClassDeclaration]
{
  Q_OBJECT

public:

  /**
   * Standard constructor.
   */
  CustomViewerWorkbenchWindowAdvisor(berry::IWorkbenchWindowConfigurer::Pointer configurer);

  /**
   * Standard destructor.
   */
  ~CustomViewerWorkbenchWindowAdvisor();

  /**
   * Customizes the workbench window, i.e. arrange a perspectives tab-bar according to the
   * PageComposite. The PageComposite is given to the WindowConfigurer for perspective related layout.
   */
  void CreateWindowContents(berry::Shell::Pointer shell);

  /**
   * For arbitrary actions after the window has been created but not yet opened.
   */
  void PostWindowCreate();

  /**
   * Menu bar, tool bar and status bar are made invisible, and the window title is being set.
   */
  void PreWindowOpen();

private Q_SLOTS:

  /**
  * Allows for runtime stylesheet update.
  */
  void UpdateStyle();
  void OpenFile();

};

#endif /*CUSTOMVIEWERWORKBENCHWINDOWADVISOR_H_*/
