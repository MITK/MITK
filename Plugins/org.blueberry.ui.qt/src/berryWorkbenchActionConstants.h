/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYWORKBENCHACTIONCONSTANTS_H
#define BERRYWORKBENCHACTIONCONSTANTS_H

#include <QString>

#include <org_blueberry_ui_qt_Export.h>

#ifdef FILE_END
// defined in winbase.h
#undef FILE_END
#endif

namespace berry {

/**
 * Action ids for standard actions, groups in the workbench menu bar, and
 * global actions.
 * <p>
 * This interface contains constants only; it is not intended to be implemented
 * or extended.
 * </p>
 * <h3>Standard menus</h3>
 * <ul>
 *   <li>File menu (<code>M_FILE</code>)</li>
 *   <li>Edit menu (<code>M_EDIT</code>)</li>
 *   <li>Window menu (<code>M_WINDOW</code>)</li>
 *   <li>Help menu (<code>M_HELP</code>)</li>
 * </ul>
 * <h3>Standard group for adding top level menus</h3>
 * <ul>
 *   <li>Extra top level menu group (<code>MB_ADDITIONS</code>)</li>
 * </ul>
 * <h3>Global actions</h3>
 * <ul>
 *   <li>Undo (<code>UNDO</code>)</li>
 *   <li>Redo (<code>REDO</code>)</li>
 *   <li>Cut (<code>CUT</code>)</li>
 *   <li>Copy (<code>COPY</code>)</li>
 *   <li>Paste (<code>PASTE</code>)</li>
 *   <li>Delete (<code>DELETE</code>)</li>
 *   <li>Find (<code>FIND</code>)</li>
 *   <li>Select All (<code>SELECT_ALL</code>)</li>
 *   <li>Add Bookmark (<code>BOOKMARK</code>)</li>
 * </ul>
 * <h3>Standard File menu actions</h3>
 * <ul>
 *   <li>Start group (<code>FILE_START</code>)</li>
 *   <li>End group (<code>FILE_END</code>)</li>
 *   <li>New action (<code>NEW</code>)</li>
 *   <li>Extra New-like action group (<code>NEW_EXT</code>)</li>
 *   <li>Close action (<code>CLOSE</code>)</li>
 *   <li>Close All action (<code>CLOSE_ALL</code>)</li>
 *   <li>Extra Close-like action group (<code>CLOSE_EXT</code>)</li>
 *   <li>Save action (<code>SAVE</code>)</li>
 *   <li>Save As action (<code>SAVE_AS</code>)</li>
 *   <li>Save All action (<code>SAVE_ALL</code>)</li>
 *   <li>Extra Save-like action group (<code>SAVE_EXT</code>)</li>
 *   <li>Import action (<code>IMPORT</code>)</li>
 *   <li>Export action (<code>EXPORT</code>)</li>
 *   <li>Extra Import-like action group (<code>IMPORT_EXT</code>)</li>
 *   <li>Quit action (<code>QUIT</code>)</li>
 * </ul>
 * <h3>Standard Edit menu actions</h3>
 * <ul>
 *   <li>Start group (<code>EDIT_START</code>)</li>
 *   <li>End group (<code>EDIT_END</code>)</li>
 *   <li>Undo global action (<code>UNDO</code>)</li>
 *   <li>Redo global action (<code>REDO</code>)</li>
 *   <li>Extra Undo-like action group (<code>UNDO_EXT</code>)</li>
 *   <li>Cut global action (<code>CUT</code>)</li>
 *   <li>Copy global action (<code>COPY</code>)</li>
 *   <li>Paste global action (<code>PASTE</code>)</li>
 *   <li>Extra Cut-like action group (<code>CUT_EXT</code>)</li>
 *   <li>Delete global action (<code>DELETE</code>)</li>
 *   <li>Find global action (<code>FIND</code>)</li>
 *   <li>Select All global action (<code>SELECT_ALL</code>)</li>
 *   <li>Bookmark global action (<code>BOOKMARK</code>)</li>
 * </ul>
 * <h3>Standard Perspective menu actions</h3>
 * <ul>
 *   <li>Extra Perspective-like action group (<code>VIEW_EXT</code>)</li>
 * </ul>
 * <h3>Standard Workbench menu actions</h3>
 * <ul>
 *   <li>Start group (<code>WB_START</code>)</li>
 *   <li>End group (<code>WB_END</code>)</li>
 *   <li>Extra Build-like action group (<code>BUILD_EXT</code>)</li>
 *   <li>Build action (<code>BUILD</code>)</li>
 *   <li>Rebuild All action (<code>REBUILD_ALL</code>)</li>
 * </ul>
 * <h3>Standard Window menu actions</h3>
 * <ul>
 *   <li>Extra Window-like action group (<code>WINDOW_EXT</code>)</li>
 * </ul>
 * <h3>Standard Help menu actions</h3>
 * <ul>
 *   <li>Start group (<code>HELP_START</code>)</li>
 *   <li>End group (<code>HELP_END</code>)</li>
 *   <li>About action (<code>ABOUT</code>)</li>
 * </ul>
 * <h3>Standard pop-up menu groups</h3>
 * <ul>
 *   <li>Managing group (<code>GROUP_MANAGING</code>)</li>
 *   <li>Reorganize group (<code>GROUP_REORGANIZE</code>)</li>
 *   <li>Add group (<code>GROUP_ADD</code>)</li>
 *   <li>File group (<code>GROUP_FILE</code>)</li>
 * </ul>
 * <p>
 * To hook a global action handler, a view should use the following code:
 * <code>
 *   IAction copyHandler = ...;
 *   view.getSite().getActionBars().setGlobalActionHandler(
 *       IWorkbenchActionConstants.COPY,
 *       copyHandler);
 * </code>
 * For editors, this should be done in the <code>IEditorActionBarContributor</code>.
 * </p>
 */
struct BERRY_UI_QT WorkbenchActionConstants
{

  // Standard area for adding top level menus:
  /**
   * Name of group for adding new top-level menus (value <code>"additions"</code>).
   */
  static const QString MB_ADDITIONS;

  /**
   * Name of standard File menu (value <code>"file"</code>).
   */
  static const QString M_FILE; // "file"

  /**
   * Name of standard Edit menu (value <code>"edit"</code>).
   */
  static const QString M_EDIT; // "edit"

  /**
   * Name of standard Window menu (value <code>"window"</code>).
   */
  static const QString M_WINDOW; // = "window"

  /**
   * Name of standard Help menu (value <code>"help"</code>).
   */
  static const QString M_HELP; // "help";

  /**
   * File menu: name of group for start of menu (value <code>"fileStart"</code>).
   */
  static const QString FILE_START; // = "fileStart"; // Group.

  /**
   * File menu: name of group for end of menu (value <code>"fileEnd"</code>).
   */
  static const QString FILE_END; // = "fileEnd"; // Group.

  /**
   * File menu: name of group for extra New-like actions (value <code>"new.ext"</code>).
   */
  static const QString NEW_EXT; // = "new.ext"; // Group.

  /**
   * File menu: name of group for extra Close-like actions (value <code>"close.ext"</code>).
   */
  static const QString CLOSE_EXT; // = "close.ext"; // Group.

  /**
   * File menu: name of group for extra Save-like actions (value <code>"save.ext"</code>).
   */
  static const QString SAVE_EXT; // = "save.ext"; // Group.

  /**
   * File menu: name of "Most Recently Used File" group.
   * (value <code>"mru"</code>).
   */
  static const QString MRU; // = "mru";

  // Standard edit actions:
  /**
   * Edit menu: name of group for start of menu (value <code>"editStart"</code>).
   */
  static const QString EDIT_START; // = "editStart"; // Group.

  /**
   * Edit menu: name of group for end of menu (value <code>"editEnd"</code>).
   */
  static const QString EDIT_END; // = "editEnd"; // Group.

  /**
   * Edit menu: name of group for extra Undo-like actions (value <code>"undo.ext"</code>).
   */
  static const QString UNDO_EXT; // = "undo.ext"; // Group.

  /**
   * Edit menu: name of group for extra Cut-like actions (value <code>"cut.ext"</code>).
   */
  static const QString CUT_EXT; // = "cut.ext"; // Group.

  /**
   * Edit menu: name of group for extra Find-like actions (value <code>"find.ext"</code>).
   */
  static const QString FIND_EXT; // = "find.ext"; // Group.

  /**
   * Edit menu: name of group for extra Add-like actions (value <code>"add.ext"</code>).
   */
  static const QString ADD_EXT; // = "add.ext"; // Group.

  // Standard help actions:
  /**
   * Help menu: name of group for start of menu
   * (value <code>"helpStart"</code>).
   */
  static const QString HELP_START; // = "helpStart"; // Group.

  /**
   * Help menu: name of group for end of menu
   * (value <code>"helpEnd"</code>).
   */
  static const QString HELP_END; // = "helpEnd"; // Group.
};

}

#endif // BERRYWORKBENCHACTIONCONSTANTS_H
