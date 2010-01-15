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
 
 ; //========================================================================*/

#ifndef CHERRYWORKBENCHPREFERENCECONSTANTS_H_
#define CHERRYWORKBENCHPREFERENCECONSTANTS_H_

#include <string>
#include "cherryUiDll.h"

namespace cherry
{

/**
 * Preference ids exposed by the openCherry Platform User Interface. These
 * preference settings can be obtained from the UI plug-in's preference store.
 * <p>
 * <b>Note:</b>This interface should not be implemented or extended.
 * </p>
 *
 * @see PlatformUI#PLUGIN_ID
 * @noextend This class is not intended to be extended by clients.
 */
struct CHERRY_UI WorkbenchPreferenceConstants
{

  /**
   * A named preference for whether to show an editor when its input file is
   * selected in the Navigator (and vice versa).
   * <p>
   * Value is of type <code>boolean</code>.
   * </p>
   */
  static const std::string LINK_NAVIGATOR_TO_EDITOR; // "LINK_NAVIGATOR_TO_EDITOR";

  /**
   * A named preference for how a new perspective is opened.
   * <p>
   * Value is of type <code>std::string</code>. The possible values are defined
   * by <code>OPEN_PERSPECTIVE_WINDOW, OPEN_PERSPECTIVE_PAGE and
   * OPEN_PERSPECTIVE_REPLACE</code>.
   * </p>
   *
   * @see #OPEN_PERSPECTIVE_WINDOW
   * @see #OPEN_PERSPECTIVE_PAGE
   * @see #OPEN_PERSPECTIVE_REPLACE
   * @see #NO_NEW_PERSPECTIVE
   */
  static const std::string OPEN_NEW_PERSPECTIVE; // "OPEN_NEW_PERSPECTIVE";

  /**
   * A preference value indicating that an action should open a new
   * perspective in a new window.
   *
   * @see #PROJECT_OPEN_NEW_PERSPECTIVE
   */
  static const std::string OPEN_PERSPECTIVE_WINDOW; // "OPEN_PERSPECTIVE_WINDOW";

  /**
   * A preference value indicating that an action should open a new
   * perspective in a new page.
   *
   * @see #PROJECT_OPEN_NEW_PERSPECTIVE
   * @deprecated Opening a Perspective in a new page is no longer supported
   *             functionality as of 2.0.
   */
  static const std::string OPEN_PERSPECTIVE_PAGE; // "OPEN_PERSPECTIVE_PAGE";

  /**
   * A preference value indicating that an action should open a new
   * perspective by replacing the current perspective.
   *
   * @see #PROJECT_OPEN_NEW_PERSPECTIVE
   */
  static const std::string OPEN_PERSPECTIVE_REPLACE; // "OPEN_PERSPECTIVE_REPLACE";

  /**
   * A preference value indicating that an action should not open a new
   * perspective.
   *
   * @see #PROJECT_OPEN_NEW_PERSPECTIVE
   */
  static const std::string NO_NEW_PERSPECTIVE; // "NO_NEW_PERSPECTIVE";

  /**
   * A named preference indicating the default workbench perspective.
   */
  static const std::string DEFAULT_PERSPECTIVE_ID; // "defaultPerspectiveId";

  /**
   * A named preference indicating the presentation factory to use for the
   * workbench look and feel.
   *
   * @since 3.0
   */
  static const std::string PRESENTATION_FACTORY_ID; // "presentationFactoryId";

  /**
   * A named preference indicating where the perspective bar should be docked.
   * The default value (when this preference is not set) is
   * <code>TOP_RIGHT</code>.
   * <p>
   * This preference may be one of the following values: {@link #TOP_RIGHT},
   * {@link #TOP_LEFT}, or {@link #LEFT}.
   * </p>
   *
   * @since 3.0
   */
  static const std::string DOCK_PERSPECTIVE_BAR; // "DOCK_PERSPECTIVE_BAR";

  /**
   * A preference indication the initial size of the perspective bar. The default value is 160.
   * This preference only works when <code>configurer.setShowPerspectiveBar(true)</code> is set in
   * WorkbenchWindowAdvisor#preWindowOpen()
   *
   * This preference only uses integer values
   * bug 84603: [RCP] [PerspectiveBar] New API or pref to set default perspective bar size
   *
   *  @since 3.5
   */
  static const std::string PERSPECTIVE_BAR_SIZE; // "PERSPECTIVE_BAR_SIZE";

  /**
   * A named preference indicating where the fast view bar should be docked in
   * a fresh workspace. This preference is meaningless after a workspace has
   * been setup, since the fast view bar state is then persisted in the
   * workbench. This preference is intended for applications that want the
   * initial docking location to be somewhere specific. The default value
   * (when this preference is not set) is the bottom.
   *
   * @see #LEFT
   * @see #BOTTOM
   * @see #RIGHT
   * @since 3.0
   */
  static const std::string INITIAL_FAST_VIEW_BAR_LOCATION; // "initialFastViewBarLocation";

  /**
   * A named preference indicating the preferred layout direction of parts
   * inside a sash container. The default value (when this preference is not set)
   * is <code>LEFT</code>.
   * <p>
   * This preference may be one of the following values: {@link #LEFT} or {@link #RIGHT}.
   * </p>
   */
  static const std::string PREFERRED_SASH_LAYOUT; // "preferredSashLayout";

  /**
   * Constant to be used when referring to the top right of the workbench
   * window.
   *
   * @see #DOCK_PERSPECTIVE_BAR
   * @since 3.0
   */
  static const std::string TOP_RIGHT; // "topRight";

  /**
   * Constant to be used when referring to the top left of the workbench
   * window.
   *
   * @see #DOCK_PERSPECTIVE_BAR
   * @since 3.0
   */
  static const std::string TOP_LEFT; // "topLeft";

  /**
   * Constant to be used when referring to the left side of the workbench
   * window.
   *
   * @see #DOCK_PERSPECTIVE_BAR
   * @see #INITIAL_FAST_VIEW_BAR_LOCATION
   * @since 3.0
   */
  static const std::string LEFT; // "left";

  /**
   * Constant to be used when referring to the bottom of the workbench window.
   *
   * @see #INITIAL_FAST_VIEW_BAR_LOCATION
   * @since 3.0
   */
  static const std::string BOTTOM; // "bottom";

  /**
   * Constant to be used when referring to the right side of the workbench
   * window.
   *
   * @see #INITIAL_FAST_VIEW_BAR_LOCATION
   * @since 3.0
   */
  static const std::string RIGHT; // "right";

  /**
   * A named preference indicating whether the workbench should show the
   * introduction component (if available) on startup.
   *
   * <p>
   * The default value for this preference is: <code>true</code> (show
   * intro)
   * </p>
   *
   * @see org.eclipse.ui.application.WorkbenchWindowAdvisor#openIntro()
   * @since 3.0
   */
  static const std::string SHOW_INTRO; // "showIntro";

  /**
   * A named preference for whether the workbench should show traditional
   * style tabs in editors and views.
   *
   * Boolean-valued: <code>true</code> if editors and views should use a
   * traditional style of tab and <code>false</code> if editors should show
   * new style tab (3.0 style)
   * <p>
   * The default value for this preference is: <code>true</code>
   * </p>
   *
   * @since 3.0
   */
  static const std::string SHOW_TRADITIONAL_STYLE_TABS; // "SHOW_TRADITIONAL_STYLE_TABS";

  /**
   * A named preference for whether the workbench should show text on the
   * perspective bar.
   *
   * Boolean-valued: <code>true</code>, if editors should show text on the
   * perspective bar, <code>false</code> otherwise.
   * <p>
   * The default value for this preference is: <code>true</code> (show text
   * on the perspective bar)
   * </p>
   *
   * @since 3.0
   */
  static const std::string SHOW_TEXT_ON_PERSPECTIVE_BAR; // "SHOW_TEXT_ON_PERSPECTIVE_BAR";

  /**
   * A named preference for whether the workbench should show the "open
   * perspective" button on the perspective bar.
   *
   * Boolean-valued: <code>true</code>, if editors should show "open
   * perspective" button on the perspective bar, <code>false</code>
   * otherwise.
   * <p>
   * The default value for this preference is: <code>true</code> (show "open
   * perspective" button on the perspective bar)
   * </p>
   *
   * @since 3.4
   */
  static const std::string SHOW_OPEN_ON_PERSPECTIVE_BAR; // "SHOW_OPEN_ON_PERSPECTIVE_BAR";

  /**
   * A named preference for whether the workbench should show the "Other..."
   * menu item in the perspective menu.
   *
   * Boolean-valued: <code>true</code>, if editors should show text on the
   * "Other..." menu item, <code>false</code> otherwise.
   * <p>
   * The default value for this preference is: <code>true</code> (show the
   * "Other..." menu item in the perspective menu)
   * </p>
   *
   * @since 3.4
   */
  static const std::string SHOW_OTHER_IN_PERSPECTIVE_MENU; // "SHOW_OTHER_IN_PERSPECTIVE_MENU";

  /**
   * A named preference for the text of the Help Contents action.
   *
   * String-valued. If not specified, <code>"&Help Contents"</code> is used.
   * <p>
   * The default value for this preference is: <code>null</code>
   * </p>
   *
   * @since 3.0
   */
  static const std::string HELP_CONTENTS_ACTION_TEXT; // "helpContentsActionText";

  /**
   * A named preference for the text of the Help Search action.
   *
   * String-valued. If not specified, <code>"S&earch"</code> is used.
   * <p>
   * The default value for this preference is: <code>null</code>
   * </p>
   *
   * @since 3.1
   */
  static const std::string HELP_SEARCH_ACTION_TEXT; // "helpSearchActionText";

  /**
   * A named preference for the text of the Dynamic Help action.
   *
   * String-valued. If not specified, <code>"&Dynamic Help"</code> is used.
   * <p>
   * The default value for this preference is: <code>null</code>
   * </p>
   *
   * @since 3.1
   */
  static const std::string DYNAMIC_HELP_ACTION_TEXT; // "dynamicHelpActionText";

  /**
   * A named preference for enabling animations when a layout transition
   * occurs
   * <p>
   * The default value for this preference is: <code>true</code> (show
   * animations when a transition occurs)
   * </p>
   *
   * @since 3.1
   */
  static const std::string ENABLE_ANIMATIONS; // "ENABLE_ANIMATIONS";

  /**
   * A named preference that view implementors can used to determine whether
   * or not they should utilize colored labels.
   *
   * <p>
   * The default value for this preference is: <code>true</code> (show
   * colored labels)
   * </p>
   *
   * @since 3.4
   */
  static const std::string USE_COLORED_LABELS; // "USE_COLORED_LABELS";

  /**
   * <p>
   * Workbench preference id for the key configuration identifier to be
   * treated as the default.
   * </p>
   * <p>
   * The default value for this preference is
   * <code>"org.eclipse.ui.defaultAcceleratorConfiguration"</code>.
   * <p>
   *
   * @since 3.1
   */
  static const std::string KEY_CONFIGURATION_ID; // "KEY_CONFIGURATION_ID";

  /**
   * <p>
   * Workbench preference identifier for the minimum width of editor tabs. By
   * default, Eclipse does not define this value and allows SWT to determine
   * this constant. We use <code>-1</code> internally to signify "use
   * default".
   * </p>
   * <p>
   * The default value for this preference is <code>-1</code>.
   * </p>
   *
   * @since 3.1
   */
  static const std::string EDITOR_MINIMUM_CHARACTERS; // "EDITOR_MINIMUM_CHARACTERS";

  /**
   * <p>
   * Workbench preference identifier for the minimum width of view tabs.
   * </p>
   * <p>
   * The default value for this preference is <code>1</code>.
   * </p>
   *
   * @since 3.2
   */
  static const std::string VIEW_MINIMUM_CHARACTERS; // "VIEW_MINIMUM_CHARACTERS";

  /**
   * Stores whether or not system jobs are being shown.
   *
   * @since 3.1
   */
  static const std::string SHOW_SYSTEM_JOBS; // "SHOW_SYSTEM_JOBS";

  /**
   * Workbench preference for the current theme.
   *
   * @since 3.1
   */
  static const std::string CURRENT_THEME_ID; // "CURRENT_THEME_ID";

  /**
   * A preference value indicating whether editors should be closed before
   * saving the workbench state when exiting. The default is
   * <code>false</code>.
   *
   * @since 3.1
   */
  static const std::string CLOSE_EDITORS_ON_EXIT; // "CLOSE_EDITORS_ON_EXIT";

  /**
   * Stores whether or not to show progress while starting the workbench. The
   * default is <code>false</code>.
   *
   * @since 3.1
   */
  static const std::string SHOW_PROGRESS_ON_STARTUP; // "SHOW_PROGRESS_ON_STARTUP";

  /**
   * Stores whether or not to show the memory monitor in the workbench window.
   *
   * @since 3.1
   */
  static const std::string SHOW_MEMORY_MONITOR; // "SHOW_MEMORY_MONITOR";

  /**
   * Stores whether or not to use the window working set as the default
   * working set for newly created views (without previously stored state).
   * This is a hint that view implementors should honor.
   *
   * @since 3.2
   */
  static const std::string USE_WINDOW_WORKING_SET_BY_DEFAULT; // "USE_WINDOW_WORKING_SET_BY_DEFAULT";

  /**
   * Stores whether or not to show the text widget that allows type-ahead
   * search in the case where a FilteredTree is used to display and filter
   * tree items.
   *
   * @since 3.2
   */
  static const std::string SHOW_FILTERED_TEXTS; // "SHOW_FILTERED_TEXTS";

  /**
   * Stores whether or not views may be detached. The default is
   * <code>true</code>.
   *
   * @since 3.2
   */
  static const std::string ENABLE_DETACHED_VIEWS; // "ENABLE_DETACHED_VIEWS";

  /**
   * Stores whether or not the workbench prompts for saving when a dirty
   * editor or view is closed, but the Saveable objects are still open in
   * other parts. If
   * <code>true<code> (default), the user will be prompted. If <code>false</code>, there will be
   * no prompt.
   *
   * @see Saveable
   * @since 3.2
   */
  static const std::string PROMPT_WHEN_SAVEABLE_STILL_OPEN; // "PROMPT_WHEN_SAVEABLE_STILL_OPEN";

  /**
   * Lists the extra perspectives to show in the perspective bar. The value is
   * a comma-separated list of perspective ids. The default is the empty
   * string.
   *
   * @since 3.2
   */
  static const std::string PERSPECTIVE_BAR_EXTRAS; // "PERSPECTIVE_BAR_EXTRAS";

  /**
   * Allows locking the trim to prevent user dragging on startup. The default
   * is <code>false</code>.
   *
   * @since 3.2
   */
  static const std::string LOCK_TRIM; // "LOCK_TRIM";

  /**
   * A named preference for providing the 3.3 presentation's min/max behaviour
   * <p>
   * The default value for this preference is: <code>false</code>; use the
   * 3.2 behaviour.
   * </p>
   *
   * @since 3.3
   */
  static const std::string ENABLE_NEW_MIN_MAX; // "ENABLE_MIN_MAX";

  /**
   * A named preference for disabling opening a new fast view from the fast
   * view bar controls ("Show View as a fast view" button or "New Fast View"
   * submenu).
   * <p>
   * Value is of type <code>boolean</code>.
   * </p>
   * <p>
   * The default is <code>false</code>.
   * </p>
   *
   * @since 3.3
   */
  static const std::string DISABLE_NEW_FAST_VIEW; // "disableNewFastView";

  /**
   * A named preference for enabling the 3.2 behavior for closing sticky
   * views. When not enabled a sticky view is closed in all perspectives when
   * the view is closed.
   * <p>
   * The default value for this preference is: <code>false</code>; use the
   * 3.2 behaviour.
   * </p>
   *
   * @since 3.3
   */
  static const std::string ENABLE_32_STICKY_CLOSE_BEHAVIOR; // "ENABLE_32_STICKY_CLOSE_BEHAVIOR";

  /**
   * An named preference for whether or not tabs are on the top or bottom
   * for views. Values are either {@link SWT#TOP} or {@link SWT#BOTTOM}.
   * <p>
   * The default value for this preference is: <code>SWT.TOP</code>.
   * </p>
   *
   * @since 3.4
   */
  static const std::string VIEW_TAB_POSITION; // "VIEW_TAB_POSITION";

  /**
   * An named preference for whether or not tabs are on the top or bottom
   * for editors. Values are either {@link SWT#TOP} or {@link SWT#BOTTOM}.
   * <p>
   * The default value for this preference is: <code>SWT.TOP</code>.
   * </p>
   *
   * @since 3.4
   */
  static const std::string EDITOR_TAB_POSITION; // "EDITOR_TAB_POSITION";

  /**
   * Workbench preference id for whether the workbench should show multiple
   * editor tabs.
   *
   * Boolean-valued: <code>true</code> if editors should show multiple
   * editor tabs, and <code>false</code> if editors should show a single
   * editor tab.
   * <p>
   * The default value for this preference is: <code>true</code>
   * </p>
   *
   * @since 3.4
   */
  static const std::string SHOW_MULTIPLE_EDITOR_TABS; // "SHOW_MULTIPLE_EDITOR_TABS";

  /**
   * Workbench preference id for whether the workbench may open editors
   * in-place. Note that editors will only be opened in-place if this
   * preference is <code>false</code> and if the current platform supports
   * in-place editing.
   *
   * Boolean-valued: <code>false</code> if editors may be opened in-place,
   * and <code>true</code> if editors should never be opened in-place.
   * <p>
   * The default value for this preference is: <code>false</code>
   * </p>
   *
   * @since 3.4
   */
  static const std::string DISABLE_OPEN_EDITOR_IN_PLACE; // "DISABLE_OPEN_EDITOR_IN_PLACE";
};

}

#endif /* CHERRYWORKBENCHPREFERENCECONSTANTS_H_ */
