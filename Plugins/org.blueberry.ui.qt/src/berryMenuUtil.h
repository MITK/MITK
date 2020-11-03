/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYMENUUTIL_H
#define BERRYMENUUTIL_H

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

/**
 * Provides utilities and constants for use with the new menus API.
 *
 * @noextend This class is not intended to be subclassed by clients.
 * @noinstantiate This class is not intended to be instantiated by clients.
 */
class BERRY_UI_QT MenuUtil
{

public:

  /**
   *
   * Workbench Menu. On supported platforms, this menu is shown when no
   * workbench windows are active
   */
  static QString WORKBENCH_MENU; // = "menu:org.blueberry.ui.workbench.menu";
  /** Main Menu */
  static QString MAIN_MENU; // = "menu:org.blueberry.ui.main.menu";
  /** Main ToolBar */
  static QString MAIN_TOOLBAR; // = "toolbar:org.blueberry.ui.main.toolbar";

  /** -Any- Popup Menu */
  static QString ANY_POPUP; // = "popup:org.blueberry.ui.popup.any";

  /**
   * Valid query attribute. Usage <b>menu:menu.id?before=contribution.id</b>.
   */
  static QString QUERY_BEFORE; // = "before";

  /**
   * Valid query attribute. Usage <b>menu:menu.id?after=contribution.id</b>.
   */
  static QString QUERY_AFTER; // = "after";

  /**
   * Valid query attribute. Usage <b>menu:menu.id?endof=contribution.id</b>.
   * <p>
   * This menu contribution will be placed at the end of the group defined by
   * <b>contribution.id</b> (usually right in front of the next group marker
   * or separator). Further contribution processing can still place other
   * contributions after this one.
   * </p>
   */
  static QString QUERY_ENDOF; // = "endof";

  /**
   * Contributions of targets to this location will be included with the show
   * in menu.
   */
  static QString SHOW_IN_MENU_ID; // = "popup:org.blueberry.ui.menus.showInMenu";

  /**
   * @param id
   *            The menu's id
   * @return The locator URI for a menu with the given id
   */
  static QString MenuUri(const QString& id);

  /**
   * @param id
   *            The id of the menu
   * @param location
   *            The relative location specifier
   * @param refId
   *            The id of the menu element to be relative to
   * @return A location URI formatted with the given parameters
   */
  static QString MenuAddition(const QString& id, const QString& location,
                              const QString& refId);

  /**
   * Convenience method to create a standard menu addition The resulting
   * string has the format: "menu:[id]?after=additions"
   *
   * @param id
   *            The id of the root element to contribute to
   * @return The formatted string
   */
  static QString MenuAddition(const QString& id);

  /**
   * @param id
   *            The toolbar's id
   * @return The lcoation URI for a toolbar with the given id
   */
  static QString ToolbarUri(const QString& id);

  /**
   * @param id
   *            The id of the toolbar
   * @param location
   *            The relative location specifier
   * @param refId
   *            The id of the toolbar element to be relative to
   * @return A location URI formatted with the given parameters
   */
  static QString ToolbarAddition(const QString& id, const QString& location,
                                 const QString& refId);

  /**
   * Convenience method to create a standard toolbar addition The resulting
   * string has the format: "toolbar:[id]?after=additions"
   *
   * @param id
   *            The id of the root element to contribute to
   * @return The formatted string
   */
  static QString ToolbarAddition(const QString& id);
};

}

#endif // BERRYMENUUTIL_H
