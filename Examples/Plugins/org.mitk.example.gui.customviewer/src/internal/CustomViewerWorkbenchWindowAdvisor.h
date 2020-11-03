/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef CUSTOMVIEWERWORKBENCHWINDOWADVISOR_H_
#define CUSTOMVIEWERWORKBENCHWINDOWADVISOR_H_

#include <berryIWorkbenchWindowConfigurer.h>
#include <berryWorkbenchWindowAdvisor.h>

#include <QmitkFileOpenAction.h>

/**
 * \brief A WorkbenchWindowAdvisor class for the custom viewer plug-in.
 *
 * This class suits the custom viewer plug-in. Menu bar, tool bar and status bar are made invisible,
 * and the window title for the custom viewer is being set. The workbench window is being customized,
 * i.e. a perspectives tab-bar is arranged according to the PageComposite. The PageComposite is then
 * laid out according to perspective related contents by the WindowConfigurer.
 *
 * @see{ CustomViewerWorkbenchWindowAdvisor::PreWindowOpen(), CustomViewerWorkbenchWindowAdvisor::CreateWindowContents()
 * }
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
  ~CustomViewerWorkbenchWindowAdvisor() override;

  /**
   * Customizes the workbench window, i.e. arrange a perspectives tab-bar according to the
   * PageComposite. The PageComposite is given to the WindowConfigurer for perspective related layout.
   */
  void CreateWindowContents(berry::Shell::Pointer shell) override;

  /**
   * For arbitrary actions after the window has been created but not yet opened.
   */
  void PostWindowCreate() override;

  /**
   * Menu bar, tool bar and status bar are made invisible, and the window title is being set.
   */
  void PreWindowOpen() override;

private Q_SLOTS:

  /**
  * Allows for runtime stylesheet update.
  */
  void UpdateStyle();
  void OpenFile();
};

#endif /*CUSTOMVIEWERWORKBENCHWINDOWADVISOR_H_*/
