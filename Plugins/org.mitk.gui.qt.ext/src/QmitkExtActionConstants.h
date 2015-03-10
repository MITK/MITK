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


#ifndef QMITKEXTACTIONCONSTANTS_H
#define QMITKEXTACTIONCONSTANTS_H

#include <berryWorkbenchActionConstants.h>

#include <org_mitk_gui_qt_ext_Export.h>

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
 *
 * @see org.eclipse.ui.IActionBars#setGlobalActionHandler
 *
 * Note: many of the remaining non-deprecated constants here are IDE-specific
 *   and should be deprecated and moved to a constant pool at the IDE layer
 *   (e.g. IIDEActionConstants).
 * @noimplement This interface is not intended to be implemented by clients.
 * @noextend This interface is not intended to be extended by clients.
 */
struct MITK_QT_COMMON_EXT_EXPORT QmitkExtActionConstants : public berry::WorkbenchActionConstants
{
  /**
   * Name of standard Window menu (value <code>"window"</code>).
   */
  static const QString M_WINDOW; // = "window"
};

#endif // QMITKEXTACTIONCONSTANTS_H
