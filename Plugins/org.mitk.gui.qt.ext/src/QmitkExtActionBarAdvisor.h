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

#ifndef QMITKEXTACTIONBARADVISOR_H_
#define QMITKEXTACTIONBARADVISOR_H_

#include <berryActionBarAdvisor.h>

#include <org_mitk_gui_qt_ext_Export.h>

namespace berry {

struct IWorkbenchWindow;

class MenuManager;

}

class MITK_QT_COMMON_EXT_EXPORT QmitkExtActionBarAdvisor : public berry::ActionBarAdvisor
{
public:

  QmitkExtActionBarAdvisor(berry::SmartPointer<berry::IActionBarConfigurer> configurer);

protected:

  void FillMenuBar(berry::IMenuManager* menuBar);

private:

  /**
   * Creates and returns the Window menu.
   */
  berry::SmartPointer<berry::MenuManager> CreateWindowMenu();

  /**
   * Adds the perspective actions to the specified menu.
   */
  void AddPerspectiveActions(berry::MenuManager* menu);

  berry::IWorkbenchWindow* window;
};

#endif /*QMITKEXTACTIONBARADVISOR_H_*/
