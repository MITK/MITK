/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYPREFERENCECONSTANTS_H_
#define BERRYPREFERENCECONSTANTS_H_

#include <QString>

namespace berry {

/**
 * The PreferenceConstants are the internal constants used by the Workbench.
 */
struct PreferenceConstants
{

public:

  //Boolean: true; // = single click opens editor; false; // = double click opens it.
  static const QString OPEN_ON_SINGLE_CLICK; // = "OPEN_ON_SINGLE_CLICK";

  //Boolean: true; // = select on hover;
  static const QString SELECT_ON_HOVER; // = "SELECT_ON_HOVER";

  //Boolean: true; // = open after delay
  static const QString OPEN_AFTER_DELAY; // = "OPEN_AFTER_DELAY";

  //Do we show color icons in toolbars?
  static const QString COLOR_ICONS; // = "COLOR_ICONS";

  //mappings for type/extension to an editor
  const static QString EDITORS; // = "editors";

  const static QString RESOURCES; // = "resourcetypes";

  //saving perspective layouts
  const static QString PERSPECTIVES; // = "perspectives";

  // (int) If > 0, an editor will be reused once 'N' editors are opened.
  static const QString REUSE_EDITORS; // = "REUSE_OPEN_EDITORS";

  //Boolean: true; // = replace dirty editor if no other editors to reuse
  // (prompt for save);
  //      false; // = open a new editor if no other editors to resuse
  static const QString REUSE_DIRTY_EDITORS; // = "REUSE_DIRTY_EDITORS";

  //On/Off option for the two preceding options.
  static const QString REUSE_EDITORS_BOOLEAN; // = "REUSE_OPEN_EDITORS_BOOLEAN";

  // (int) N recently viewed files will be listed in the File->Open Recent
  // menu.
  static const QString RECENT_FILES; // = "RECENT_FILES";

  // (integer) Mode for opening a view.
  static const QString OPEN_VIEW_MODE; // = "OPEN_VIEW_MODE";

  static const int OVM_EMBED; // = 0;

  static const int OVM_FAST; // = 1;

  static const int OVM_FLOAT; // = 2;

  // (int) Mode for opening a new perspective
  static const QString OPEN_PERSP_MODE; // = "OPEN_PERSPECTIVE_MODE";

  static const int OPM_ACTIVE_PAGE; // = 0;

  // static const int OPM_NEW_PAGE; // = 1;
  static const int OPM_NEW_WINDOW; // = 2;

  //Identifier for enabled decorators
  static const QString ENABLED_DECORATORS; // = "ENABLED_DECORATORS";

  //Boolean: true; // = keep cycle part dialog open when keys released
  static const QString STICKY_CYCLE; // = "STICKY_CYCLE";

  //List of plugins but that extends "startup" extension point but are
  // overriden by the user.
  //QString of plugin unique ids separated by ";"
  static const QString PLUGINS_NOT_ACTIVATED_ON_STARTUP; // = "PLUGINS_NOT_ACTIVATED_ON_STARTUP";

  //Separator for PLUGINS_NOT_ACTIVATED_ON_STARTUP
  static const char SEPARATOR; // = ';';

  //Preference key for default editors
  const static QString DEFAULT_EDITORS; // = "defaultEditors";

  //Preference key for default editors
  const static QString DEFAULT_EDITORS_CACHE; // = "defaultEditorsCache";

  //Tab width; // = tab height * scalar value
  const static QString EDITOR_TAB_WIDTH; // = "EDITOR_TAB_WIDTH";

  //Boolean: true; // = show Editors drop down button on CTabFolder
  static const QString EDITORLIST_PULLDOWN_ACTIVE; // = "EDITORLIST_PULLDOWN_ACTIVE";

  // Selection scope for EditorList
  static const QString EDITORLIST_SELECTION_SCOPE; // = "EDITORLIST_SELECTION_SCOPE";

  static const int EDITORLIST_SET_WINDOW_SCOPE; // = 0;

  static const int EDITORLIST_SET_PAGE_SCOPE; // = 1;

  static const int EDITORLIST_SET_TAB_GROUP_SCOPE; // = 2;

  // Sort criteria for EditorList
  static const QString EDITORLIST_SORT_CRITERIA; // = "EDITORLIST_SORT_CRITERIA";

  static const int EDITORLIST_NAME_SORT; // = 0;

  static const int EDITORLIST_MRU_SORT; // = 1;

  /**
   * Boolean; true; // = EditorList displays full path
   */
  static const QString EDITORLIST_DISPLAY_FULL_NAME; // = "EDITORLIST_DISPLAY_FULL_NAME";


  /**
   * Workbench preference id for determining whether the user has chosen to
   * override some of the settings in the current presentation.
   * <p>
   * The default value for this preference is: <code>false</code> (prompt)
   * </p>
   *
   */
  static const QString OVERRIDE_PRESENTATION; // = "overridepresentation"; //$

  /**
   * <p>
   * The key for the preference indicating which tab is selected in the keys
   * preference page when last okay was pressed. This value should never
   * really be directly edited by a user.
   * </p>
   * <p>
   * This preference is an <code>int</code> value. The default value is
   * <code>0</code>.
   * </p>
   *
   */
  static const QString KEYS_PREFERENCE_SELECTED_TAB; // = "KEYS_PREFERENCE_SELECTED_TAB";

  /**
   * <p>
   * The key for the preference indicating whether multi-stroke key sequences
   * should provide assistance to the user. This means that if the user pauses
   * after pressing the first key, a window will open showing the possible
   * completions.
   * </p>
   * <p>
   * This preference is a <code>boolean</code> value. The default value is
   * <code>false</code>.
   * </p>
   *
   */
  static const QString MULTI_KEY_ASSIST; // = "MULTI_KEY_ASSIST";

  /**
   * <p>
   * The key for the preference indicating how long the assist window should
   * wait before opening. This is a value in milliseconds -- from the time the
   * first key in a multi-key is received by the system, to the time the
   * assist window should appear.
   * </p>
   * <p>
   * This preference is an <code>int</code> value. The default value is
   * <code>1000</code>.
   * </p>
   */
  static const QString MULTI_KEY_ASSIST_TIME; // = "MULTI_KEY_ASSIST_TIME";

  /**
   * Workbench preference to use the new IPersistableEditor interface
   * throughout the workbench new editor/open editor calls.
   */
  static const QString USE_IPERSISTABLE_EDITORS; // = "USE_IPERSISTABLE_EDITORS";

  /**
   * Preference to show user jobs in a dialog.
   */
  static const QString RUN_IN_BACKGROUND; // = "RUN_IN_BACKGROUND";

  /**
   * Workbench preference id for determining whether the user will be prompted
   * for activity enablement. If this is false then activities are enabled
   * automatically. If it is true, then the user is only prompted for
   * activities that they have not already declared a disinterest in via the
   * prompt dialog.
   * <p>
   * The default value for this preference is: <code>true</code> (prompt)
   * </p>
   */
  static const QString SHOULD_PROMPT_FOR_ENABLEMENT; // = "shouldPromptForEnablement";

};

}

#endif /* BERRYPREFERENCECONSTANTS_H_ */
