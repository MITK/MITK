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

#include "cherryWorkbenchConstants.h"

#include "../cherryPlatformUI.h"

namespace cherry
{

const std::string WorkbenchConstants::DEFAULT_PRESENTATION_ID =
    "org.opencherry.ui.presentations.default"; //$NON-NLS-1$

const std::string WorkbenchConstants::RESOURCE_TYPE_FILE_NAME =
    "resourcetypes.xml"; //$NON-NLS-1$

// Filename containing the workbench's preferences
const std::string WorkbenchConstants::PREFERENCE_BUNDLE_FILE_NAME =
    "workbench.ini"; //$NON-NLS-1$

// Identifier for visible view parts.
const std::string WorkbenchConstants::WORKBENCH_VISIBLE_VIEW_ID =
    "Workbench.visibleViewID"; //$NON-NLS-1$

// Identifier of workbench info properties page
//const std::string WorkbenchConstants::WORKBENCH_PROPERTIES_PAGE_INFO =
//    PlatformUI::PLUGIN_ID + ".propertypages.info.file"; //$NON-NLS-1$

// Various editor.
//const std::string WorkbenchConstants::OLE_EDITOR_ID = PlatformUI::PLUGIN_ID
//    + ".OleEditor"; //$NON-NLS-1$

// Default view category.
const std::string WorkbenchConstants::DEFAULT_CATEGORY_ID = "org.opencherry.ui";
    //PlatformUI::PLUGIN_ID;

// Persistance tags.
const std::string WorkbenchConstants::TRUE_VAL = "true"; //$NON-NLS-1$

const std::string WorkbenchConstants::FALSE_VAL = "false"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_WORKBENCH_ADVISOR =
    "workbenchAdvisor"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_WORKBENCH_WINDOW_ADVISOR =
    "workbenchWindowAdvisor"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_ACTION_BAR_ADVISOR =
    "actionBarAdvisor"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_ID = "id"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_FOCUS = "focus"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_EDITOR = "editor"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_DEFAULT_EDITOR = "defaultEditor"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_DELETED_EDITOR = "deletedEditor"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_EDITORS = "editors"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_WORKBOOK = "workbook"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_ACTIVE_WORKBOOK = "activeWorkbook"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_AREA = "editorArea"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_AREA_VISIBLE = "editorAreaVisible"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_AREA_HIDDEN = "editorAreaHidden"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_AREA_TRIM_STATE =
    "editorAreaTrimState"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_INPUT = "input"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_FACTORY_ID = "factoryID"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_EDITOR_STATE = "editorState"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_TITLE = "title"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_X = "x"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_Y = "y"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_FLOAT = "float"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_ITEM_WRAP_INDEX = "wrapIndex"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_TOOLBAR_LAYOUT = "toolbarLayout"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_WIDTH = "width"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_HEIGHT = "height"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_MINIMIZED = "minimized"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_MAXIMIZED = "maximized"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_FOLDER = "folder"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_INFO = "info"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_PART = "part"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_PART_NAME = "partName"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_PROPERTIES = "properties"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_PROPERTY = "property"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_RELATIVE = "relative"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_RELATIONSHIP = "relationship"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_RATIO = "ratio"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_RATIO_LEFT = "ratioLeft"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_RATIO_RIGHT = "ratioRight"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_ACTIVE_PAGE_ID = "activePageID"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_EXPANDED = "expanded"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_PAGE = "page"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_INTRO = "intro"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_STANDBY = "standby"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_LABEL = "label"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_CONTENT = "content"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_CLASS = "class"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_FILE = "file"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_DESCRIPTOR = "descriptor"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_MAIN_WINDOW = "mainWindow"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_DETACHED_WINDOW = "detachedWindow"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_HIDDEN_WINDOW = "hiddenWindow"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_WORKBENCH = "workbench"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_WINDOW = "window"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_VERSION = "version"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_PROGRESS_COUNT = "progressCount"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_PERSPECTIVES = "perspectives"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_PERSPECTIVE = "perspective"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_ACTIVE_PERSPECTIVE =
    "activePerspective"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_ACTIVE_PART = "activePart"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_ACTION_SET = "actionSet"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_ALWAYS_ON_ACTION_SET =
    "alwaysOnActionSet"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_ALWAYS_OFF_ACTION_SET =
    "alwaysOffActionSet"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_SHOW_VIEW_ACTION = "show_view_action"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_SHOW_IN_TIME = "show_in_time"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_TIME = "time"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_NEW_WIZARD_ACTION =
    "new_wizard_action"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_PERSPECTIVE_ACTION =
    "perspective_action"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_VIEW = "view"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_LAYOUT = "layout"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_EXTENSION = "extension"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_NAME = "name"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_IMAGE = "image"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_LAUNCHER = "launcher"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_PLUGIN = "plugin"; //$NON-NLS-1$

/** deprecated - use TAG_OPEN_MODE */
const std::string WorkbenchConstants::TAG_INTERNAL = "internal"; //$NON-NLS-1$

/** deprecated - use TAG_OPEN_MODE */
const std::string WorkbenchConstants::TAG_OPEN_IN_PLACE = "open_in_place"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_PROGRAM_NAME = "program_name"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_FAST_VIEWS = "fastViews"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_FAST_VIEW_BAR = "fastViewBar"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_FAST_VIEW_BARS = "fastViewBars"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_GLOBAL_FAST_VIEWS = "globalFastViews"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_FAST_GROUPS = "fastGroups"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_FIXED = "fixed";//$NON-NLS-1$

const std::string WorkbenchConstants::TAG_CLOSEABLE = "closeable";//$NON-NLS-1$

const std::string WorkbenchConstants::TAG_MOVEABLE = "moveable";//$NON-NLS-1$

const std::string WorkbenchConstants::TAG_APPEARANCE = "appearance"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_PRESENTATION = "presentation"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_STANDALONE = "standalone";//$NON-NLS-1$

const std::string WorkbenchConstants::TAG_SHOW_TITLE = "showTitle";//$NON-NLS-1$

const std::string WorkbenchConstants::TAG_VIEW_STATE = "viewState"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_SINGLETON = "singleton"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_EDITOR_REUSE_THRESHOLD =
    "editorReuseThreshold"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_PERSISTABLE = "persistable"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_MRU_LIST = "mruList"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_PERSPECTIVE_HISTORY = "perspHistory"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_WORKING_SET_MANAGER =
    "workingSetManager"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_WORKING_SETS = "workingSets"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_WORKING_SET = "workingSet"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_ITEM = "item"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_EDIT_PAGE_ID = "editPageId"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_COOLBAR_LAYOUT = "coolbarLayout"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_ITEM_SIZE = "itemSize"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_ITEM_X = "x"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_ITEM_Y = "y"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_ITEM_TYPE = "itemType"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_TYPE_SEPARATOR = "typeSeparator"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_TYPE_GROUPMARKER = "typeGroupMarker"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_TYPE_TOOLBARCONTRIBUTION =
    "typeToolBarContribution"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_TYPE_PLACEHOLDER = "typePlaceholder"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_COOLITEM = "coolItem"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_INDEX = "index"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_PINNED = "pinned"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_PATH = "path";//$NON-NLS-1$

const std::string WorkbenchConstants::TAG_TOOLTIP = "tooltip";//$NON-NLS-1$

const std::string WorkbenchConstants::TAG_VIEWS = "views";//$NON-NLS-1$

const std::string WorkbenchConstants::TAG_POSITION = "position";//$NON-NLS-1$

const std::string WorkbenchConstants::TAG_NAVIGATION_HISTORY =
    "navigationHistory";//$NON-NLS-1$

const std::string WorkbenchConstants::TAG_STICKY_STATE = "stickyState"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_ACTIVE = "active";//$NON-NLS-1$

const std::string WorkbenchConstants::TAG_REMOVED = "removed";//$NON-NLS-1$

const std::string WorkbenchConstants::TAG_HISTORY_LABEL = "historyLabel";//$NON-NLS-1$

const std::string WorkbenchConstants::TAG_LOCKED = "locked";//$NON-NLS-1$

const std::string WorkbenchConstants::TAG_OPEN_MODE = "openMode"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_STARTUP = "startup"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_FAST_VIEW_SIDE = "fastViewLocation"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_FAST_VIEW_DATA = "fastViewData"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_FAST_VIEW_ORIENTATION = "orientation"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_FAST_VIEW_SEL_ID = "selectedTabId"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_FAST_VIEW_STYLE = "style"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_THEME = "theme";//$NON-NLS-1$

const std::string WorkbenchConstants::TAG_VIEW_LAYOUT_REC = "viewLayoutRec"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_PERSPECTIVE_BAR = "perspectiveBar"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_TRIM = "trimLayout"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_TRIM_AREA = "trimArea"; //$NON-NLS-1$

const std::string WorkbenchConstants::TAG_TRIM_ITEM = "trimItem"; //$NON-NLS-1$

//Fonts
const std::string WorkbenchConstants::SMALL_FONT = "org.opencherry.ui.smallFont"; //$NON-NLS-1$

//Colors
const std::string WorkbenchConstants::COLOR_HIGHLIGHT_ =
    "org.opencherry.ui.highlight"; //$NON-NLS-1$

}
