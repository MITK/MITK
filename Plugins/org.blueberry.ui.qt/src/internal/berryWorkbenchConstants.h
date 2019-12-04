/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYWORKBENCHCONSTANTS_H_
#define BERRYWORKBENCHCONSTANTS_H_

#include <QString>

#include <org_blueberry_ui_qt_Export.h>

namespace berry
{

/**
 * General constants used by the workbench.
 */
struct BERRY_UI_QT WorkbenchConstants
{

  static const QString DEFAULT_PRESENTATION_ID; // = "org.blueberry.ui.presentations.default";

  static const QString RESOURCE_TYPE_FILE_NAME; // = "resourcetypes.xml";

  // Filename containing the workbench's preferences
  static const QString PREFERENCE_BUNDLE_FILE_NAME; // = "workbench.ini";

  // Identifier for visible view parts.
  static const QString WORKBENCH_VISIBLE_VIEW_ID; // = "Workbench.visibleViewID";

  // Identifier of workbench info properties page
  static const QString WORKBENCH_PROPERTIES_PAGE_INFO; // = PlatformUI.PLUGIN_ID + ".propertypages.info.file";

  // Various editor.
  static const QString OLE_EDITOR_ID; // = PlatformUI.PLUGIN_ID + ".OleEditor";

  // Default view category.
  static const QString DEFAULT_CATEGORY_ID; // = PlatformUI.PLUGIN_ID;

  // Persistance tags.
  static const QString TRUE_VAL; // = "true";

  static const QString FALSE_VAL; // = "false";

  static const QString TAG_WORKBENCH_ADVISOR; // = "workbenchAdvisor";

  static const QString TAG_WORKBENCH_WINDOW_ADVISOR; // = "workbenchWindowAdvisor";

  static const QString TAG_ACTION_BAR_ADVISOR; // = "actionBarAdvisor";

  static const QString TAG_ID; // = "id";

  static const QString TAG_FOCUS; // = "focus";

  static const QString TAG_EDITOR; // = "editor";

  static const QString TAG_DEFAULT_EDITOR; // = "defaultEditor";

  static const QString TAG_DELETED_EDITOR; // = "deletedEditor";

  static const QString TAG_EDITORS; // = "editors";

  static const QString TAG_WORKBOOK; // = "workbook";

  static const QString TAG_ACTIVE_WORKBOOK; // = "activeWorkbook";

  static const QString TAG_AREA; // = "editorArea";

  static const QString TAG_AREA_VISIBLE; // = "editorAreaVisible";

  static const QString TAG_AREA_HIDDEN; // = "editorAreaHidden";

  static const QString TAG_AREA_TRIM_STATE; // = "editorAreaTrimState";

  static const QString TAG_INPUT; // = "input";

  static const QString TAG_FACTORY_ID; // = "factoryID";

  static const QString TAG_EDITOR_STATE; // = "editorState";

  static const QString TAG_TITLE; // = "title";

  static const QString TAG_X; // = "x";

  static const QString TAG_Y; // = "y";

  static const QString TAG_FLOAT; // = "float";

  static const QString TAG_ITEM_WRAP_INDEX; // = "wrapIndex";

  static const QString TAG_TOOLBAR_LAYOUT; // = "toolbarLayout";

  static const QString TAG_WIDTH; // = "width";

  static const QString TAG_HEIGHT; // = "height";

  static const QString TAG_MINIMIZED; // = "minimized";

  static const QString TAG_MAXIMIZED; // = "maximized";

  static const QString TAG_FOLDER; // = "folder";

  static const QString TAG_INFO; // = "info";

  static const QString TAG_PART; // = "part";

  static const QString TAG_PART_NAME; // = "partName";

  static const QString TAG_PROPERTIES; // = "properties";

  static const QString TAG_PROPERTY; // = "property";

  static const QString TAG_RELATIVE; // = "relative";

  static const QString TAG_RELATIONSHIP; // = "relationship";

  static const QString TAG_RATIO; // = "ratio";

  static const QString TAG_RATIO_LEFT; // = "ratioLeft";

  static const QString TAG_RATIO_RIGHT; // = "ratioRight";

  static const QString TAG_ACTIVE_PAGE_ID; // = "activePageID";

  static const QString TAG_EXPANDED; // = "expanded";

  static const QString TAG_PAGE; // = "page";

  static const QString TAG_INTRO; // = "intro";

  static const QString TAG_STANDBY; // = "standby";

  static const QString TAG_LABEL; // = "label";

  static const QString TAG_CONTENT; // = "content";

  static const QString TAG_CLASS; // = "class";

  static const QString TAG_FILE; // = "file";

  static const QString TAG_DESCRIPTOR; // = "descriptor";

  static const QString TAG_MAIN_WINDOW; // = "mainWindow";

  static const QString TAG_DETACHED_WINDOW; // = "detachedWindow";

  static const QString TAG_HIDDEN_WINDOW; // = "hiddenWindow";

  static const QString TAG_WORKBENCH; // = "workbench";

  static const QString TAG_WINDOW; // = "window";

  static const QString TAG_VERSION; // = "version";

  static const QString TAG_PROGRESS_COUNT; // = "progressCount";

  static const QString TAG_PERSPECTIVES; // = "perspectives";

  static const QString TAG_PERSPECTIVE; // = "perspective";

  static const QString TAG_ACTIVE_PERSPECTIVE; // = "activePerspective";

  static const QString TAG_ACTIVE_PART; // = "activePart";

  static const QString TAG_ACTION_SET; // = "actionSet";

  static const QString TAG_ALWAYS_ON_ACTION_SET; // = "alwaysOnActionSet";

  static const QString TAG_ALWAYS_OFF_ACTION_SET; // = "alwaysOffActionSet";

  static const QString TAG_SHOW_VIEW_ACTION; // = "show_view_action";

  static const QString TAG_SHOW_IN_TIME; // = "show_in_time";

  static const QString TAG_TIME; // = "time";

  static const QString TAG_NEW_WIZARD_ACTION; // = "new_wizard_action";

  static const QString TAG_PERSPECTIVE_ACTION; // = "perspective_action";

  static const QString TAG_VIEW; // = "view";

  static const QString TAG_LAYOUT; // = "layout";

  static const QString TAG_EXTENSION; // = "extension";

  static const QString TAG_NAME; // = "name";

  static const QString TAG_IMAGE; // = "image";

  static const QString TAG_LAUNCHER; // = "launcher";

  static const QString TAG_PLUGIN; // = "plugin";

  /** deprecated - use TAG_OPEN_MODE */
  static const QString TAG_INTERNAL; // = "internal";

  /** deprecated - use TAG_OPEN_MODE */
  static const QString TAG_OPEN_IN_PLACE; // = "open_in_place";

  static const QString TAG_PROGRAM_NAME; // = "program_name";

  static const QString TAG_FAST_VIEWS; // = "fastViews";

  static const QString TAG_FAST_VIEW_BAR; // = "fastViewBar";

  static const QString TAG_FAST_VIEW_BARS; // = "fastViewBars";

  static const QString TAG_GLOBAL_FAST_VIEWS; // = "globalFastViews";

  static const QString TAG_FAST_GROUPS; // = "fastGroups";

  static const QString TAG_FIXED; // = "fixed";//$NON-NLS-1$

  static const QString TAG_CLOSEABLE; // = "closeable";//$NON-NLS-1$

  static const QString TAG_MOVEABLE; // = "moveable";//$NON-NLS-1$

  static const QString TAG_APPEARANCE; // = "appearance";

  static const QString TAG_PRESENTATION; // = "presentation";

  static const QString TAG_STANDALONE; // = "standalone";//$NON-NLS-1$

  static const QString TAG_SHOW_TITLE; // = "showTitle";//$NON-NLS-1$

  static const QString TAG_VIEW_STATE; // = "viewState";

  static const QString TAG_SINGLETON; // = "singleton";

  static const QString TAG_EDITOR_REUSE_THRESHOLD; // = "editorReuseThreshold";

  static const QString TAG_PERSISTABLE; // = "persistable";

  static const QString TAG_MRU_LIST; // = "mruList";

  static const QString TAG_PERSPECTIVE_HISTORY; // = "perspHistory";

  static const QString TAG_WORKING_SET_MANAGER; // = "workingSetManager";

  static const QString TAG_WORKING_SETS; // = "workingSets";

  static const QString TAG_WORKING_SET; // = "workingSet";

  static const QString TAG_ITEM; // = "item";

  static const QString TAG_EDIT_PAGE_ID; // = "editPageId";

  static const QString TAG_COOLBAR_LAYOUT; // = "coolbarLayout";

  static const QString TAG_ITEM_SIZE; // = "itemSize";

  static const QString TAG_ITEM_X; // = "x";

  static const QString TAG_ITEM_Y; // = "y";

  static const QString TAG_ITEM_TYPE; // = "itemType";

  static const QString TAG_TYPE_SEPARATOR; // = "typeSeparator";

  static const QString TAG_TYPE_GROUPMARKER; // = "typeGroupMarker";

  static const QString TAG_TYPE_TOOLBARCONTRIBUTION; // = "typeToolBarContribution";

  static const QString TAG_TYPE_PLACEHOLDER; // = "typePlaceholder";

  static const QString TAG_COOLITEM; // = "coolItem";

  static const QString TAG_INDEX; // = "index";

  static const QString TAG_PINNED; // = "pinned";

  static const QString TAG_PATH; // = "path";//$NON-NLS-1$

  static const QString TAG_TOOLTIP; // = "tooltip";//$NON-NLS-1$

  static const QString TAG_VIEWS; // = "views";//$NON-NLS-1$

  static const QString TAG_POSITION; // = "position";//$NON-NLS-1$

  static const QString TAG_NAVIGATION_HISTORY; // = "navigationHistory";//$NON-NLS-1$

  static const QString TAG_STICKY_STATE; // = "stickyState";

  static const QString TAG_ACTIVE; // = "active";//$NON-NLS-1$

  static const QString TAG_REMOVED; // = "removed";//$NON-NLS-1$

  static const QString TAG_HISTORY_LABEL; // = "historyLabel";//$NON-NLS-1$

  static const QString TAG_LOCKED; // = "locked";//$NON-NLS-1$

  static const QString TAG_OPEN_MODE; // = "openMode";

  static const QString TAG_STARTUP; // = "startup";

  static const QString TAG_FAST_VIEW_SIDE; // = "fastViewLocation";

  static const QString TAG_FAST_VIEW_DATA; // = "fastViewData";

  static const QString TAG_FAST_VIEW_ORIENTATION; // = "orientation";

  static const QString TAG_FAST_VIEW_SEL_ID; // = "selectedTabId";

  static const QString TAG_FAST_VIEW_STYLE; // = "style";

  static const QString TAG_THEME; // = "theme";//$NON-NLS-1$

  static const QString TAG_VIEW_LAYOUT_REC; // = "viewLayoutRec";

  static const QString TAG_PERSPECTIVE_BAR; // = "perspectiveBar";

  static const QString TAG_TRIM; // = "trimLayout";

  static const QString TAG_TRIM_AREA; // = "trimArea";

  static const QString TAG_TRIM_ITEM; // = "trimItem";

  //Fonts
  static const QString SMALL_FONT; // = "org.blueberry.ui.smallFont";

  //Colors
  static const QString COLOR_HIGHLIGHT_; // = "org.blueberry.ui.highlight";

};

}

#endif /* BERRYWORKBENCHCONSTANTS_H_ */
