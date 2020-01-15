/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryWorkbenchConstants.h"

#include "berryPlatformUI.h"

namespace berry
{

const QString WorkbenchConstants::DEFAULT_PRESENTATION_ID =
    "org.blueberry.ui.presentations.default";

const QString WorkbenchConstants::RESOURCE_TYPE_FILE_NAME =
    "resourcetypes.xml";

// Filename containing the workbench's preferences
const QString WorkbenchConstants::PREFERENCE_BUNDLE_FILE_NAME =
    "workbench.ini";

// Identifier for visible view parts.
const QString WorkbenchConstants::WORKBENCH_VISIBLE_VIEW_ID =
    "Workbench.visibleViewID";

// Identifier of workbench info properties page
//const QString WorkbenchConstants::WORKBENCH_PROPERTIES_PAGE_INFO =
//    PlatformUI::PLUGIN_ID + ".propertypages.info.file";

// Various editor.
//const QString WorkbenchConstants::OLE_EDITOR_ID = PlatformUI::PLUGIN_ID
//    + ".OleEditor";

// Default view category.
const QString WorkbenchConstants::DEFAULT_CATEGORY_ID = "org.blueberry.ui";
    //PlatformUI::PLUGIN_ID;

// Persistance tags.
const QString WorkbenchConstants::TRUE_VAL = "true";

const QString WorkbenchConstants::FALSE_VAL = "false";

const QString WorkbenchConstants::TAG_WORKBENCH_ADVISOR =
    "workbenchAdvisor";

const QString WorkbenchConstants::TAG_WORKBENCH_WINDOW_ADVISOR =
    "workbenchWindowAdvisor";

const QString WorkbenchConstants::TAG_ACTION_BAR_ADVISOR =
    "actionBarAdvisor";

const QString WorkbenchConstants::TAG_ID = "id";

const QString WorkbenchConstants::TAG_FOCUS = "focus";

const QString WorkbenchConstants::TAG_EDITOR = "editor";

const QString WorkbenchConstants::TAG_DEFAULT_EDITOR = "defaultEditor";

const QString WorkbenchConstants::TAG_DELETED_EDITOR = "deletedEditor";

const QString WorkbenchConstants::TAG_EDITORS = "editors";

const QString WorkbenchConstants::TAG_WORKBOOK = "workbook";

const QString WorkbenchConstants::TAG_ACTIVE_WORKBOOK = "activeWorkbook";

const QString WorkbenchConstants::TAG_AREA = "editorArea";

const QString WorkbenchConstants::TAG_AREA_VISIBLE = "editorAreaVisible";

const QString WorkbenchConstants::TAG_AREA_HIDDEN = "editorAreaHidden";

const QString WorkbenchConstants::TAG_AREA_TRIM_STATE =
    "editorAreaTrimState";

const QString WorkbenchConstants::TAG_INPUT = "input";

const QString WorkbenchConstants::TAG_FACTORY_ID = "factoryID";

const QString WorkbenchConstants::TAG_EDITOR_STATE = "editorState";

const QString WorkbenchConstants::TAG_TITLE = "title";

const QString WorkbenchConstants::TAG_X = "x";

const QString WorkbenchConstants::TAG_Y = "y";

const QString WorkbenchConstants::TAG_FLOAT = "float";

const QString WorkbenchConstants::TAG_ITEM_WRAP_INDEX = "wrapIndex";

const QString WorkbenchConstants::TAG_TOOLBAR_LAYOUT = "toolbarLayout";

const QString WorkbenchConstants::TAG_WIDTH = "width";

const QString WorkbenchConstants::TAG_HEIGHT = "height";

const QString WorkbenchConstants::TAG_MINIMIZED = "minimized";

const QString WorkbenchConstants::TAG_MAXIMIZED = "maximized";

const QString WorkbenchConstants::TAG_FOLDER = "folder";

const QString WorkbenchConstants::TAG_INFO = "info";

const QString WorkbenchConstants::TAG_PART = "part";

const QString WorkbenchConstants::TAG_PART_NAME = "partName";

const QString WorkbenchConstants::TAG_PROPERTIES = "properties";

const QString WorkbenchConstants::TAG_PROPERTY = "property";

const QString WorkbenchConstants::TAG_RELATIVE = "relative";

const QString WorkbenchConstants::TAG_RELATIONSHIP = "relationship";

const QString WorkbenchConstants::TAG_RATIO = "ratio";

const QString WorkbenchConstants::TAG_RATIO_LEFT = "ratioLeft";

const QString WorkbenchConstants::TAG_RATIO_RIGHT = "ratioRight";

const QString WorkbenchConstants::TAG_ACTIVE_PAGE_ID = "activePageID";

const QString WorkbenchConstants::TAG_EXPANDED = "expanded";

const QString WorkbenchConstants::TAG_PAGE = "page";

const QString WorkbenchConstants::TAG_INTRO = "intro";

const QString WorkbenchConstants::TAG_STANDBY = "standby";

const QString WorkbenchConstants::TAG_LABEL = "label";

const QString WorkbenchConstants::TAG_CONTENT = "content";

const QString WorkbenchConstants::TAG_CLASS = "class";

const QString WorkbenchConstants::TAG_FILE = "file";

const QString WorkbenchConstants::TAG_DESCRIPTOR = "descriptor";

const QString WorkbenchConstants::TAG_MAIN_WINDOW = "mainWindow";

const QString WorkbenchConstants::TAG_DETACHED_WINDOW = "detachedWindow";

const QString WorkbenchConstants::TAG_HIDDEN_WINDOW = "hiddenWindow";

const QString WorkbenchConstants::TAG_WORKBENCH = "workbench";

const QString WorkbenchConstants::TAG_WINDOW = "window";

const QString WorkbenchConstants::TAG_VERSION = "version";

const QString WorkbenchConstants::TAG_PROGRESS_COUNT = "progressCount";

const QString WorkbenchConstants::TAG_PERSPECTIVES = "perspectives";

const QString WorkbenchConstants::TAG_PERSPECTIVE = "perspective";

const QString WorkbenchConstants::TAG_ACTIVE_PERSPECTIVE =
    "activePerspective";

const QString WorkbenchConstants::TAG_ACTIVE_PART = "activePart";

const QString WorkbenchConstants::TAG_ACTION_SET = "actionSet";

const QString WorkbenchConstants::TAG_ALWAYS_ON_ACTION_SET =
    "alwaysOnActionSet";

const QString WorkbenchConstants::TAG_ALWAYS_OFF_ACTION_SET =
    "alwaysOffActionSet";

const QString WorkbenchConstants::TAG_SHOW_VIEW_ACTION = "show_view_action";

const QString WorkbenchConstants::TAG_SHOW_IN_TIME = "show_in_time";

const QString WorkbenchConstants::TAG_TIME = "time";

const QString WorkbenchConstants::TAG_NEW_WIZARD_ACTION =
    "new_wizard_action";

const QString WorkbenchConstants::TAG_PERSPECTIVE_ACTION =
    "perspective_action";

const QString WorkbenchConstants::TAG_VIEW = "view";

const QString WorkbenchConstants::TAG_LAYOUT = "layout";

const QString WorkbenchConstants::TAG_EXTENSION = "extension";

const QString WorkbenchConstants::TAG_NAME = "name";

const QString WorkbenchConstants::TAG_IMAGE = "image";

const QString WorkbenchConstants::TAG_LAUNCHER = "launcher";

const QString WorkbenchConstants::TAG_PLUGIN = "plugin";

/** deprecated - use TAG_OPEN_MODE */
const QString WorkbenchConstants::TAG_INTERNAL = "internal";

/** deprecated - use TAG_OPEN_MODE */
const QString WorkbenchConstants::TAG_OPEN_IN_PLACE = "open_in_place";

const QString WorkbenchConstants::TAG_PROGRAM_NAME = "program_name";

const QString WorkbenchConstants::TAG_FAST_VIEWS = "fastViews";

const QString WorkbenchConstants::TAG_FAST_VIEW_BAR = "fastViewBar";

const QString WorkbenchConstants::TAG_FAST_VIEW_BARS = "fastViewBars";

const QString WorkbenchConstants::TAG_GLOBAL_FAST_VIEWS = "globalFastViews";

const QString WorkbenchConstants::TAG_FAST_GROUPS = "fastGroups";

const QString WorkbenchConstants::TAG_FIXED = "fixed";//$NON-NLS-1$

const QString WorkbenchConstants::TAG_CLOSEABLE = "closeable";//$NON-NLS-1$

const QString WorkbenchConstants::TAG_MOVEABLE = "moveable";//$NON-NLS-1$

const QString WorkbenchConstants::TAG_APPEARANCE = "appearance";

const QString WorkbenchConstants::TAG_PRESENTATION = "presentation";

const QString WorkbenchConstants::TAG_STANDALONE = "standalone";//$NON-NLS-1$

const QString WorkbenchConstants::TAG_SHOW_TITLE = "showTitle";//$NON-NLS-1$

const QString WorkbenchConstants::TAG_VIEW_STATE = "viewState";

const QString WorkbenchConstants::TAG_SINGLETON = "singleton";

const QString WorkbenchConstants::TAG_EDITOR_REUSE_THRESHOLD =
    "editorReuseThreshold";

const QString WorkbenchConstants::TAG_PERSISTABLE = "persistable";

const QString WorkbenchConstants::TAG_MRU_LIST = "mruList";

const QString WorkbenchConstants::TAG_PERSPECTIVE_HISTORY = "perspHistory";

const QString WorkbenchConstants::TAG_WORKING_SET_MANAGER =
    "workingSetManager";

const QString WorkbenchConstants::TAG_WORKING_SETS = "workingSets";

const QString WorkbenchConstants::TAG_WORKING_SET = "workingSet";

const QString WorkbenchConstants::TAG_ITEM = "item";

const QString WorkbenchConstants::TAG_EDIT_PAGE_ID = "editPageId";

const QString WorkbenchConstants::TAG_COOLBAR_LAYOUT = "coolbarLayout";

const QString WorkbenchConstants::TAG_ITEM_SIZE = "itemSize";

const QString WorkbenchConstants::TAG_ITEM_X = "x";

const QString WorkbenchConstants::TAG_ITEM_Y = "y";

const QString WorkbenchConstants::TAG_ITEM_TYPE = "itemType";

const QString WorkbenchConstants::TAG_TYPE_SEPARATOR = "typeSeparator";

const QString WorkbenchConstants::TAG_TYPE_GROUPMARKER = "typeGroupMarker";

const QString WorkbenchConstants::TAG_TYPE_TOOLBARCONTRIBUTION =
    "typeToolBarContribution";

const QString WorkbenchConstants::TAG_TYPE_PLACEHOLDER = "typePlaceholder";

const QString WorkbenchConstants::TAG_COOLITEM = "coolItem";

const QString WorkbenchConstants::TAG_INDEX = "index";

const QString WorkbenchConstants::TAG_PINNED = "pinned";

const QString WorkbenchConstants::TAG_PATH = "path";//$NON-NLS-1$

const QString WorkbenchConstants::TAG_TOOLTIP = "tooltip";//$NON-NLS-1$

const QString WorkbenchConstants::TAG_VIEWS = "views";//$NON-NLS-1$

const QString WorkbenchConstants::TAG_POSITION = "position";//$NON-NLS-1$

const QString WorkbenchConstants::TAG_NAVIGATION_HISTORY =
    "navigationHistory";//$NON-NLS-1$

const QString WorkbenchConstants::TAG_STICKY_STATE = "stickyState";

const QString WorkbenchConstants::TAG_ACTIVE = "active";//$NON-NLS-1$

const QString WorkbenchConstants::TAG_REMOVED = "removed";//$NON-NLS-1$

const QString WorkbenchConstants::TAG_HISTORY_LABEL = "historyLabel";//$NON-NLS-1$

const QString WorkbenchConstants::TAG_LOCKED = "locked";//$NON-NLS-1$

const QString WorkbenchConstants::TAG_OPEN_MODE = "openMode";

const QString WorkbenchConstants::TAG_STARTUP = "startup";

const QString WorkbenchConstants::TAG_FAST_VIEW_SIDE = "fastViewLocation";

const QString WorkbenchConstants::TAG_FAST_VIEW_DATA = "fastViewData";

const QString WorkbenchConstants::TAG_FAST_VIEW_ORIENTATION = "orientation";

const QString WorkbenchConstants::TAG_FAST_VIEW_SEL_ID = "selectedTabId";

const QString WorkbenchConstants::TAG_FAST_VIEW_STYLE = "style";

const QString WorkbenchConstants::TAG_THEME = "theme";//$NON-NLS-1$

const QString WorkbenchConstants::TAG_VIEW_LAYOUT_REC = "viewLayoutRec";

const QString WorkbenchConstants::TAG_PERSPECTIVE_BAR = "perspectiveBar";

const QString WorkbenchConstants::TAG_TRIM = "trimLayout";

const QString WorkbenchConstants::TAG_TRIM_AREA = "trimArea";

const QString WorkbenchConstants::TAG_TRIM_ITEM = "trimItem";

//Fonts
const QString WorkbenchConstants::SMALL_FONT = "org.blueberry.ui.smallFont";

//Colors
const QString WorkbenchConstants::COLOR_HIGHLIGHT_ =
    "org.blueberry.ui.highlight";

}
