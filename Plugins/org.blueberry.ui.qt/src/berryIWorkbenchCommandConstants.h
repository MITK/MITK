/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYIWORKBENCHCOMMANDCONSTANTS_H
#define BERRYIWORKBENCHCOMMANDCONSTANTS_H

#include <org_blueberry_ui_qt_Export.h>

#include <QString>

namespace berry {

/**
 * Constants for all commands defined by the BlueBerry workbench.
 *
 * @noextend This interface is not intended to be extended by clients.
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI_QT IWorkbenchCommandConstants {

  // File Category

  /**
   * Id for command "New" in category "File"
   * (value is <code>"org.blueberry.ui.newWizard"</code>).
   */
  static const QString FILE_NEW; // = "org.blueberry.ui.newWizard";

  /**
   * Id for command "Open File" in category "File"
   * (value is <code>"org.blueberry.ui.file.openLocal"</code>).
   */
  static const QString FILE_OPEN; // = "org.blueberry.ui.file.openLocal";

  /**
   * Id for command "Save" in category "File"
   * (value is <code>"org.blueberry.ui.file.save"</code>).
   */
  static const QString FILE_SAVE; // = "org.blueberry.ui.file.save";

  /**
   * Id for command "Exit" in category "File"
   * (value is <code>"org.blueberry.ui.file.exit"</code>).
   */
  static const QString FILE_EXIT; // = "org.blueberry.ui.file.exit";

  // Edit Category:

  /**
   * Id for command "Undo" in category "Edit"
   * (value is <code>"org.blueberry.ui.edit.undo"</code>).
   */
  static const QString EDIT_UNDO; // = "org.blueberry.ui.edit.undo";

  /**
   * Id for command "Redo" in category "Edit"
   * (value is <code>"org.blueberry.ui.edit.redo"</code>).
   */
  static const QString EDIT_REDO; // = "org.blueberry.ui.edit.redo";

  /**
   * Id for command "Cut" in category "Edit"
   * (value is <code>"org.blueberry.ui.edit.cut"</code>).
   */
  static const QString EDIT_CUT; // = "org.blueberry.ui.edit.cut";

  /**
   * Id for command "Copy" in category "Edit"
   * (value is <code>"org.blueberry.ui.edit.copy"</code>).
   */
  static const QString EDIT_COPY; // = "org.blueberry.ui.edit.copy";

  /**
   * Id for command "Paste" in category "Edit"
   * (value is <code>"org.blueberry.ui.edit.paste"</code>).
   */
  static const QString EDIT_PASTE; // = "org.blueberry.ui.edit.paste";

  /**
   * Id for command "Delete" in category "Edit"
   * (value is <code>"org.blueberry.ui.edit.delete"</code>).
   */
  static const QString EDIT_DELETE; // = "org.blueberry.ui.edit.delete";

  // Window Category:

  /**
   * Id for command "New Window" in category "Window"
   * (value is <code>"org.eclipse.ui.window.newWindow"</code>).
   */
  static const QString WINDOW_NEW_WINDOW; // = "org.blueberry.ui.window.newWindow";

  /**
   * Id for command "New Editor" in category "Window"
   * (value is <code>"org.eclipse.ui.window.newEditor"</code>).
   */
  static const QString WINDOW_NEW_EDITOR; // = "org.blueberry.ui.window.newEditor";

  /**
   * Id for command "Show View Menu" in category "Window"
   * (value is <code>"org.eclipse.ui.window.showViewMenu"</code>).
   */
  static const QString WINDOW_SHOW_VIEW_MENU; // = "org.blueberry.ui.window.showViewMenu";

  /**
   * Id for command "Activate Editor" in category "Window"
   * (value is <code>"org.eclipse.ui.window.activateEditor"</code>).
   */
  static const QString WINDOW_ACTIVATE_EDITOR; // = "org.blueberry.ui.window.activateEditor";

  /**
   * Id for command "Maximize Active View or Editor" in category "Window"
   * (value is <code>"org.eclipse.ui.window.maximizePart"</code>).
   */
  static const QString WINDOW_MAXIMIZE_ACTIVE_VIEW_OR_EDITOR; // = "org.blueberry.ui.window.maximizePart";

  /**
   * Id for command "Minimize Active View or Editor" in category "Window"
   * (value is <code>"org.eclipse.ui.window.minimizePart"</code>).
   */
  static const QString WINDOW_MINIMIZE_ACTIVE_VIEW_OR_EDITOR; // = "org.blueberry.ui.window.minimizePart";

  /**
   * Id for command "Next Editor" in category "Window"
   * (value is <code>"org.eclipse.ui.window.nextEditor"</code>).
   */
  static const QString WINDOW_NEXT_EDITOR; // = "org.blueberry.ui.window.nextEditor";

  /**
   * Id for command "Previous Editor" in category "Window"
   * (value is <code>"org.eclipse.ui.window.previousEditor"</code>).
   */
  static const QString WINDOW_PREVIOUS_EDITOR; // = "org.blueberry.ui.window.previousEditor";

  /**
   * Id for command "Next View" in category "Window"
   * (value is <code>"org.eclipse.ui.window.nextView"</code>).
   */
  static const QString WINDOW_NEXT_VIEW; // = "org.blueberry.ui.window.nextView";

  /**
   * Id for command "Previous View" in category "Window"
   * (value is <code>"org.eclipse.ui.window.previousView"</code>).
   */
  static const QString WINDOW_PREVIOUS_VIEW; // = "org.blueberry.ui.window.previousView";

  /**
   * Id for command "Next Perspective" in category "Window"
   * (value is <code>"org.eclipse.ui.window.nextPerspective"</code>).
   */
  static const QString WINDOW_NEXT_PERSPECTIVE; // = "org.blueberry.ui.window.nextPerspective";

  /**
   * Id for command "Previous Perspective" in category "Window"
   * (value is <code>"org.eclipse.ui.window.previousPerspective"</code>).
   */
  static const QString WINDOW_PREVIOUS_PERSPECTIVE; // = "org.blueberry.ui.window.previousPerspective";

  /**
   * Id for command "Close All Perspectives" in category "Window"
   * (value is <code>"org.eclipse.ui.window.closeAllPerspectives"</code>).
   */
  static const QString WINDOW_CLOSE_ALL_PERSPECTIVES; // = "org.blueberry.ui.window.closeAllPerspectives";

  /**
   * Id for command "Close Perspective" in category "Window"
   * (value is <code>"org.eclipse.ui.window.closePerspective"</code>).
   */
  static const QString WINDOW_CLOSE_PERSPECTIVE; // = "org.blueberry.ui.window.closePerspective";

  /**
   * Id for parameter "Perspective Id" in command "Close Perspective" in
   * category "Window" (value is
   * <code>"org.eclipse.ui.window.closePerspective.perspectiveId"</code>).
   * Optional.
   */
  static const QString WINDOW_CLOSE_PERSPECTIVE_PARM_ID; // = "org.blueberry.ui.window.closePerspective.perspectiveId";

  /**
   * Id for command "Close Part" in category "Window" (value is
   * <code>"org.eclipse.ui.file.closePart"</code>).
   */
  static const QString WINDOW_CLOSE_PART; // = "org.blueberry.ui.file.closePart";

  /**
   * Id for command "Customize Perspective" in category "Window"
   * (value is <code>"org.eclipse.ui.window.customizePerspective"</code>).
   */
  static const QString WINDOW_CUSTOMIZE_PERSPECTIVE; // = "org.blueberry.ui.window.customizePerspective";

  /**
   * Id for command "Pin Editor" in category "Window"
   * (value is <code>"org.eclipse.ui.window.pinEditor"</code>).
   */
  static const QString WINDOW_PIN_EDITOR; // = "org.blueberry.ui.window.pinEditor";

  /**
   * Id for command "Preferences" in category "Window"
   * (value is <code>"org.eclipse.ui.window.preferences"</code>).
   */
  static const QString WINDOW_PREFERENCES; // = "org.blueberry.ui.window.preferences";

  /**
   * Id for parameter "Preference Page Id" in command "Preferences" in
   * category "Window" (value is <code>"preferencePageId"</code>). Optional.
   */
  static const QString WINDOW_PREFERENCES_PARM_PAGEID; // = "preferencePageId";

  /**
   * Id for command "Reset Perspective" in category "Window" (value is
   * <code>"org.eclipse.ui.window.resetPerspective"</code>).
   */
  static const QString WINDOW_RESET_PERSPECTIVE; // = "org.blueberry.ui.window.resetPerspective";

  /**
   * Id for command "Save Perspective As" in category "Window"
   * (value is <code>"org.eclipse.ui.window.savePerspective"</code>).
   */
  static const QString WINDOW_SAVE_PERSPECTIVE_AS; // = "org.blueberry.ui.window.savePerspective";

  /**
   * Id for command "Show Key Assist" in category "Window"
   * (value is <code>"org.eclipse.ui.window.showKeyAssist"</code>).
   */
  static const QString WINDOW_SHOW_KEY_ASSIST; // = "org.blueberry.ui.window.showKeyAssist";

  // Help Category:

  /**
   * Id for command "Help Contents" in category "Help"
   * (value is <code>"org.blueberry.ui.help.helpContents"</code>).
   */
  static const QString HELP_HELP_CONTENTS; // = "org.blueberry.ui.help.helpContents";

  /**
   * Id for command "Help Search" in category "Help"
   * (value is <code>"org.blueberry.ui.help.helpSearch"</code>).
   */
  static const QString HELP_HELP_SEARCH; // = "org.blueberry.ui.help.helpSearch";

  /**
   * Id for command "Dynamic Help" in category "Help"
   * (value is <code>"org.blueberry.ui.help.dynamicHelp"</code>).
   */
  static const QString HELP_DYNAMIC_HELP; // = "org.blueberry.ui.help.dynamicHelp";

  /**
   * Id for command "Welcome" in category "Help"
   * (value is <code>"org.blueberry.ui.help.quickStartAction"</code>).
   */
  static const QString HELP_WELCOME; // = "org.blueberry.ui.help.intro";

  /**
   * Id for command "About" in category "Help"
   * (value is <code>"org.blueberry.ui.help.aboutAction"</code>).
   */
  static const QString HELP_ABOUT; // = "org.blueberry.ui.help.aboutAction";

  // Views Category:

  /**
   * Id for command "Show View" in category "Views"
   * (value is <code>"org.blueberry.ui.views.showView"</code>).
   */
  static const QString VIEWS_SHOW_VIEW; // = "org.blueberry.ui.views.showView";

   /**
    * Id for parameter "View Id" in command "Show View" in category "Views"
    * (value is <code>"org.blueberry.ui.views.showView.viewId"</code>).
    */
   static const QString VIEWS_SHOW_VIEW_PARM_ID; // = "org.blueberry.ui.views.showView.viewId";

   /**
    * Id for parameter "Secondary Id" in command "Show View" in category "Views"
    * (value is <code>"org.blueberry.ui.views.showView.secondaryId"</code>).
    */
   static const QString VIEWS_SHOW_VIEW_SECONDARY_ID; // = "org.blueberry.ui.views.showView.secondaryId";

   /**
    * Id for parameter "As Fastview" in command "Show View" in category "Views"
    * (value is <code>"org.blueberry.ui.views.showView.makeFast"</code>).
    * Optional.
    */
   static const QString VIEWS_SHOW_VIEW_PARM_FASTVIEW; // = "org.blueberry.ui.views.showView.makeFast";

   // Perspectives Category:

   /**
    * Id for command "Show Perspective" in category "Perspectives"
    * (value is <code>"org.blueberry.ui.perspectives.showPerspective"</code>).
    */
   static const QString PERSPECTIVES_SHOW_PERSPECTIVE; // = "org.blueberry.ui.perspectives.showPerspective";

   /**
    * Id for parameter "Perspective Id" in command "Show Perspective" in
    * category "Perspectives" (value is
    * <code>"org.blueberry.ui.perspectives.showPerspective.perspectiveId"</code>
    * ).
    */
   static const QString PERSPECTIVES_SHOW_PERSPECTIVE_PARM_ID; // = "org.blueberry.ui.perspectives.showPerspective.perspectiveId";

   /**
    * Id for parameter "In New Window" in command "Show Perspective" in
    * category "Perspectives" (value is
    * <code>"org.blueberry.ui.perspectives.showPerspective.newWindow"</code>).
    * Optional.
    */
   static const QString PERSPECTIVES_SHOW_PERSPECTIVE_PARM_NEWWINDOW; // = "org.blueberry.ui.perspectives.showPerspective.newWindow";

};

}

#endif // BERRYIWORKBENCHCOMMANDCONSTANTS_H
