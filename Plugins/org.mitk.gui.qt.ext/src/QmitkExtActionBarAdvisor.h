/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKEXTACTIONBARADVISOR_H_
#define QMITKEXTACTIONBARADVISOR_H_

#include <berryActionBarAdvisor.h>

#include <org_mitk_gui_qt_ext_Export.h>

#include <QIcon>
#include <QKeySequence>

namespace berry {

struct IContributionItem;
struct IWorkbenchWindow;

class MenuManager;

}

class MITK_QT_COMMON_EXT_EXPORT QmitkExtActionBarAdvisor : public berry::ActionBarAdvisor
{
public:

  QmitkExtActionBarAdvisor(berry::SmartPointer<berry::IActionBarConfigurer> configurer);

protected:

  void MakeActions(berry::IWorkbenchWindow* window) override;

  void FillMenuBar(berry::IMenuManager* menuBar) override;

  void FillToolBar(berry::IToolBarManager* toolBar) override;

private:

  /**
   * Creates and returns the File menu.
   */
  berry::SmartPointer<berry::MenuManager> CreateFileMenu();

  /**
   * Creates and returns the Edit menu.
   */
  berry::SmartPointer<berry::MenuManager> CreateEditMenu();

  /**
   * Creates and returns the Window menu.
   */
  berry::SmartPointer<berry::MenuManager> CreateWindowMenu();

  /**
   * Creates and returns the Help menu.
   */
  berry::SmartPointer<berry::MenuManager> CreateHelpMenu();


  /**
   * Adds the perspective actions to the specified menu.
   */
  void AddPerspectiveActions(berry::MenuManager* menu);

  berry::SmartPointer<berry::IContributionItem> GetItem(const QString& commandId, const QString& label,
                                                        const QString& tooltip = QString(), const QIcon& icon = QIcon(),
                                                        const QKeySequence& shortcut = QKeySequence());

  berry::IWorkbenchWindow* window;
};

#endif /*QMITKEXTACTIONBARADVISOR_H_*/
