/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYISERVICESCOPES_H
#define BERRYISERVICESCOPES_H

#include <QString>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

/**
 * Different levels of service locators supported by the workbench.
 *
 * @noextend This interface is not intended to be extended by clients.
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI_QT IServiceScopes
{
  /**
   * The global service locator scope.
   */
  static const QString WORKBENCH_SCOPE; // = "org.blueberry.ui.IWorkbench";

  /**
   * A sub-scope to the global scope that is not the workbench window.
   */
  static const QString DIALOG_SCOPE; // = "org.blueberry.ui.IDialog";

  /**
   * A workbench window service locator scope.
   */
  static const QString WINDOW_SCOPE; // = "org.blueberry.ui.IWorkbenchWindow";

  /**
   * A part site service locator scope.  Found in editors and views.
   */
  static const QString PARTSITE_SCOPE; // = "org.blueberry.ui.IWorkbenchPartSite";

  /**
   * A page site service locator scope.  Found in pages in a PageBookView.
   */
  static const QString PAGESITE_SCOPE; // = "org.blueberry.ui.PageSite";

  /**
   * An editor site within a MultiPageEditorPart.
   */
  static const QString MPESITE_SCOPE; // = "org.blueberry.ui.MultiPageEditorSite";

private:

  IServiceScopes();
};

}

#endif // BERRYISERVICESCOPES_H
