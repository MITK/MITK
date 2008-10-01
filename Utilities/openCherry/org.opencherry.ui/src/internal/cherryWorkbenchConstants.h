/*=========================================================================

 Program:   openCherry Platform
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

#ifndef CHERRYWORKBENCHCONSTANTS_H_
#define CHERRYWORKBENCHCONSTANTS_H_

#include <string>

namespace cherry
{

/**
 * General constants used by the workbench.
 */
struct WorkbenchConstants
{

  static const std::string DEFAULT_PRESENTATION_ID; // = "org.opencherry.ui.presentations.default"; //$NON-NLS-1$

  static const std::string RESOURCE_TYPE_FILE_NAME; // = "resourcetypes.xml"; //$NON-NLS-1$

  // Filename containing the workbench's preferences
  static const std::string PREFERENCE_BUNDLE_FILE_NAME; // = "workbench.ini"; //$NON-NLS-1$

  // Identifier for visible view parts.
  static const std::string WORKBENCH_VISIBLE_VIEW_ID; // = "Workbench.visibleViewID"; //$NON-NLS-1$

  // Identifier of workbench info properties page
  static const std::string WORKBENCH_PROPERTIES_PAGE_INFO; // = PlatformUI.PLUGIN_ID + ".propertypages.info.file"; //$NON-NLS-1$

  // Various editor.
  static const std::string OLE_EDITOR_ID; // = PlatformUI.PLUGIN_ID + ".OleEditor"; //$NON-NLS-1$

  // Default view category.
  static const std::string DEFAULT_CATEGORY_ID; // = PlatformUI.PLUGIN_ID;

  // Persistance tags.
  static const std::string TRUE_VAL; // = "true"; //$NON-NLS-1$

  static const std::string FALSE_VAL; // = "false"; //$NON-NLS-1$

  static const std::string TAG_WORKBENCH_ADVISOR; // = "workbenchAdvisor"; //$NON-NLS-1$

  static const std::string TAG_WORKBENCH_WINDOW_ADVISOR; // = "workbenchWindowAdvisor"; //$NON-NLS-1$

  static const std::string TAG_ACTION_BAR_ADVISOR; // = "actionBarAdvisor"; //$NON-NLS-1$

  static const std::string TAG_ID; // = "id"; //$NON-NLS-1$

  static const std::string TAG_FOCUS; // = "focus"; //$NON-NLS-1$

  static const std::string TAG_EDITOR; // = "editor"; //$NON-NLS-1$

  static const std::string TAG_DEFAULT_EDITOR; // = "defaultEditor"; //$NON-NLS-1$

  static const std::string TAG_DELETED_EDITOR; // = "deletedEditor"; //$NON-NLS-1$

  static const std::string TAG_EDITORS; // = "editors"; //$NON-NLS-1$

  static const std::string TAG_WORKBOOK; // = "workbook"; //$NON-NLS-1$

  static const std::string TAG_ACTIVE_WORKBOOK; // = "activeWorkbook"; //$NON-NLS-1$

  static const std::string TAG_AREA; // = "editorArea"; //$NON-NLS-1$

  static const std::string TAG_AREA_VISIBLE; // = "editorAreaVisible"; //$NON-NLS-1$

  static const std::string TAG_AREA_HIDDEN; // = "editorAreaHidden"; //$NON-NLS-1$

  static const std::string TAG_AREA_TRIM_STATE; // = "editorAreaTrimState"; //$NON-NLS-1$

  static const std::string TAG_INPUT; // = "input"; //$NON-NLS-1$

  static const std::string TAG_FACTORY_ID; // = "factoryID"; //$NON-NLS-1$

  static const std::string TAG_EDITOR_STATE; // = "editorState"; //$NON-NLS-1$

  static const std::string TAG_TITLE; // = "title"; //$NON-NLS-1$

  static const std::string TAG_X; // = "x"; //$NON-NLS-1$

  static const std::string TAG_Y; // = "y"; //$NON-NLS-1$

  static const std::string TAG_FLOAT; // = "float"; //$NON-NLS-1$

  static const std::string TAG_ITEM_WRAP_INDEX; // = "wrapIndex"; //$NON-NLS-1$

  static const std::string TAG_TOOLBAR_LAYOUT; // = "toolbarLayout"; //$NON-NLS-1$

  static const std::string TAG_WIDTH; // = "width"; //$NON-NLS-1$

  static const std::string TAG_HEIGHT; // = "height"; //$NON-NLS-1$

  static const std::string TAG_MINIMIZED; // = "minimized"; //$NON-NLS-1$

  static const std::string TAG_MAXIMIZED; // = "maximized"; //$NON-NLS-1$

  static const std::string TAG_FOLDER; // = "folder"; //$NON-NLS-1$

  static const std::string TAG_INFO; // = "info"; //$NON-NLS-1$

  static const std::string TAG_PART; // = "part"; //$NON-NLS-1$

  static const std::string TAG_PART_NAME; // = "partName"; //$NON-NLS-1$

  static const std::string TAG_PROPERTIES; // = "properties"; //$NON-NLS-1$

  static const std::string TAG_PROPERTY; // = "property"; //$NON-NLS-1$

  static const std::string TAG_RELATIVE; // = "relative"; //$NON-NLS-1$

  static const std::string TAG_RELATIONSHIP; // = "relationship"; //$NON-NLS-1$

  static const std::string TAG_RATIO; // = "ratio"; //$NON-NLS-1$

  static const std::string TAG_RATIO_LEFT; // = "ratioLeft"; //$NON-NLS-1$

  static const std::string TAG_RATIO_RIGHT; // = "ratioRight"; //$NON-NLS-1$

  static const std::string TAG_ACTIVE_PAGE_ID; // = "activePageID"; //$NON-NLS-1$

  static const std::string TAG_EXPANDED; // = "expanded"; //$NON-NLS-1$

  static const std::string TAG_PAGE; // = "page"; //$NON-NLS-1$

  static const std::string TAG_INTRO; // = "intro"; //$NON-NLS-1$

  static const std::string TAG_STANDBY; // = "standby"; //$NON-NLS-1$

  static const std::string TAG_LABEL; // = "label"; //$NON-NLS-1$

  static const std::string TAG_CONTENT; // = "content"; //$NON-NLS-1$

  static const std::string TAG_CLASS; // = "class"; //$NON-NLS-1$

  static const std::string TAG_FILE; // = "file"; //$NON-NLS-1$

  static const std::string TAG_DESCRIPTOR; // = "descriptor"; //$NON-NLS-1$

  static const std::string TAG_MAIN_WINDOW; // = "mainWindow"; //$NON-NLS-1$

  static const std::string TAG_DETACHED_WINDOW; // = "detachedWindow"; //$NON-NLS-1$

  static const std::string TAG_HIDDEN_WINDOW; // = "hiddenWindow"; //$NON-NLS-1$

  static const std::string TAG_WORKBENCH; // = "workbench"; //$NON-NLS-1$

  static const std::string TAG_WINDOW; // = "window"; //$NON-NLS-1$

  static const std::string TAG_VERSION; // = "version"; //$NON-NLS-1$

  static const std::string TAG_PROGRESS_COUNT; // = "progressCount";  //$NON-NLS-1$

  static const std::string TAG_PERSPECTIVES; // = "perspectives"; //$NON-NLS-1$

  static const std::string TAG_PERSPECTIVE; // = "perspective"; //$NON-NLS-1$

  static const std::string TAG_ACTIVE_PERSPECTIVE; // = "activePerspective"; //$NON-NLS-1$

  static const std::string TAG_ACTIVE_PART; // = "activePart"; //$NON-NLS-1$

  static const std::string TAG_ACTION_SET; // = "actionSet"; //$NON-NLS-1$

  static const std::string TAG_ALWAYS_ON_ACTION_SET; // = "alwaysOnActionSet"; //$NON-NLS-1$

  static const std::string TAG_ALWAYS_OFF_ACTION_SET; // = "alwaysOffActionSet"; //$NON-NLS-1$

  static const std::string TAG_SHOW_VIEW_ACTION; // = "show_view_action"; //$NON-NLS-1$

  static const std::string TAG_SHOW_IN_TIME; // = "show_in_time"; //$NON-NLS-1$

  static const std::string TAG_TIME; // = "time"; //$NON-NLS-1$

  static const std::string TAG_NEW_WIZARD_ACTION; // = "new_wizard_action"; //$NON-NLS-1$

  static const std::string TAG_PERSPECTIVE_ACTION; // = "perspective_action"; //$NON-NLS-1$

  static const std::string TAG_VIEW; // = "view"; //$NON-NLS-1$

  static const std::string TAG_LAYOUT; // = "layout"; //$NON-NLS-1$

  static const std::string TAG_EXTENSION; // = "extension"; //$NON-NLS-1$

  static const std::string TAG_NAME; // = "name"; //$NON-NLS-1$

  static const std::string TAG_IMAGE; // = "image"; //$NON-NLS-1$

  static const std::string TAG_LAUNCHER; // = "launcher"; //$NON-NLS-1$

  static const std::string TAG_PLUGIN; // = "plugin"; //$NON-NLS-1$

  /** deprecated - use TAG_OPEN_MODE */
  static const std::string TAG_INTERNAL; // = "internal"; //$NON-NLS-1$

  /** deprecated - use TAG_OPEN_MODE */
  static const std::string TAG_OPEN_IN_PLACE; // = "open_in_place"; //$NON-NLS-1$

  static const std::string TAG_PROGRAM_NAME; // = "program_name"; //$NON-NLS-1$

  static const std::string TAG_FAST_VIEWS; // = "fastViews"; //$NON-NLS-1$

  static const std::string TAG_FAST_VIEW_BAR; // = "fastViewBar"; //$NON-NLS-1$

  static const std::string TAG_FAST_VIEW_BARS; // = "fastViewBars"; //$NON-NLS-1$

  static const std::string TAG_GLOBAL_FAST_VIEWS; // = "globalFastViews"; //$NON-NLS-1$

  static const std::string TAG_FAST_GROUPS; // = "fastGroups"; //$NON-NLS-1$

  static const std::string TAG_FIXED; // = "fixed";//$NON-NLS-1$

  static const std::string TAG_CLOSEABLE; // = "closeable";//$NON-NLS-1$

  static const std::string TAG_MOVEABLE; // = "moveable";//$NON-NLS-1$

  static const std::string TAG_APPEARANCE; // = "appearance"; //$NON-NLS-1$

  static const std::string TAG_PRESENTATION; // = "presentation"; //$NON-NLS-1$

  static const std::string TAG_STANDALONE; // = "standalone";//$NON-NLS-1$

  static const std::string TAG_SHOW_TITLE; // = "showTitle";//$NON-NLS-1$

  static const std::string TAG_VIEW_STATE; // = "viewState"; //$NON-NLS-1$

  static const std::string TAG_SINGLETON; // = "singleton"; //$NON-NLS-1$

  static const std::string TAG_EDITOR_REUSE_THRESHOLD; // = "editorReuseThreshold"; //$NON-NLS-1$

  static const std::string TAG_PERSISTABLE; // = "persistable"; //$NON-NLS-1$

  static const std::string TAG_MRU_LIST; // = "mruList"; //$NON-NLS-1$

  static const std::string TAG_PERSPECTIVE_HISTORY; // = "perspHistory"; //$NON-NLS-1$

  static const std::string TAG_WORKING_SET_MANAGER; // = "workingSetManager"; //$NON-NLS-1$

  static const std::string TAG_WORKING_SETS; // = "workingSets"; //$NON-NLS-1$

  static const std::string TAG_WORKING_SET; // = "workingSet"; //$NON-NLS-1$

  static const std::string TAG_ITEM; // = "item"; //$NON-NLS-1$

  static const std::string TAG_EDIT_PAGE_ID; // = "editPageId"; //$NON-NLS-1$

  static const std::string TAG_COOLBAR_LAYOUT; // = "coolbarLayout"; //$NON-NLS-1$

  static const std::string TAG_ITEM_SIZE; // = "itemSize"; //$NON-NLS-1$

  static const std::string TAG_ITEM_X; // = "x"; //$NON-NLS-1$

  static const std::string TAG_ITEM_Y; // = "y"; //$NON-NLS-1$

  static const std::string TAG_ITEM_TYPE; // = "itemType"; //$NON-NLS-1$

  static const std::string TAG_TYPE_SEPARATOR; // = "typeSeparator"; //$NON-NLS-1$

  static const std::string TAG_TYPE_GROUPMARKER; // = "typeGroupMarker"; //$NON-NLS-1$

  static const std::string TAG_TYPE_TOOLBARCONTRIBUTION; // = "typeToolBarContribution"; //$NON-NLS-1$

  static const std::string TAG_TYPE_PLACEHOLDER; // = "typePlaceholder"; //$NON-NLS-1$

  static const std::string TAG_COOLITEM; // = "coolItem"; //$NON-NLS-1$

  static const std::string TAG_INDEX; // = "index"; //$NON-NLS-1$

  static const std::string TAG_PINNED; // = "pinned"; //$NON-NLS-1$

  static const std::string TAG_PATH; // = "path";//$NON-NLS-1$

  static const std::string TAG_TOOLTIP; // = "tooltip";//$NON-NLS-1$

  static const std::string TAG_VIEWS; // = "views";//$NON-NLS-1$

  static const std::string TAG_POSITION; // = "position";//$NON-NLS-1$

  static const std::string TAG_NAVIGATION_HISTORY; // = "navigationHistory";//$NON-NLS-1$

  static const std::string TAG_STICKY_STATE; // = "stickyState"; //$NON-NLS-1$

  static const std::string TAG_ACTIVE; // = "active";//$NON-NLS-1$

  static const std::string TAG_REMOVED; // = "removed";//$NON-NLS-1$

  static const std::string TAG_HISTORY_LABEL; // = "historyLabel";//$NON-NLS-1$

  static const std::string TAG_LOCKED; // = "locked";//$NON-NLS-1$

  static const std::string TAG_OPEN_MODE; // = "openMode"; //$NON-NLS-1$

  static const std::string TAG_STARTUP; // = "startup"; //$NON-NLS-1$

  static const std::string TAG_FAST_VIEW_SIDE; // = "fastViewLocation"; //$NON-NLS-1$

  static const std::string TAG_FAST_VIEW_DATA; // = "fastViewData"; //$NON-NLS-1$

  static const std::string TAG_FAST_VIEW_ORIENTATION; // = "orientation"; //$NON-NLS-1$

  static const std::string TAG_FAST_VIEW_SEL_ID; // = "selectedTabId"; //$NON-NLS-1$

  static const std::string TAG_FAST_VIEW_STYLE; // = "style"; //$NON-NLS-1$

  static const std::string TAG_THEME; // = "theme";//$NON-NLS-1$

  static const std::string TAG_VIEW_LAYOUT_REC; // = "viewLayoutRec"; //$NON-NLS-1$

  static const std::string TAG_PERSPECTIVE_BAR; // = "perspectiveBar"; //$NON-NLS-1$

  static const std::string TAG_TRIM; // = "trimLayout"; //$NON-NLS-1$

  static const std::string TAG_TRIM_AREA; // = "trimArea"; //$NON-NLS-1$

  static const std::string TAG_TRIM_ITEM; // = "trimItem"; //$NON-NLS-1$

  //Fonts
  static const std::string SMALL_FONT; // = "org.opencherry.ui.smallFont"; //$NON-NLS-1$

  //Colors
  static const std::string COLOR_HIGHLIGHT; // = "org.opencherry.ui.highlight"; //$NON-NLS-1$

};

}

#endif /* CHERRYWORKBENCHCONSTANTS_H_ */
