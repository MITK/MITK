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

#ifndef CHERRYISOURCES_H_
#define CHERRYISOURCES_H_

#include <string>
#include "cherryUiDll.h"

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 *
 * <p>
 * A source is type of event change that can occur within the workbench. For
 * example, the active workbench window can change, so it is considered a
 * source. Workbench services can track changes to these sources, and thereby
 * try to resolve conflicts between a variety of possible options. This is most
 * commonly used for things like handlers and contexts.
 * </p>
 * <p>
 * This interface defines the source that are known to the workbench at
 * compile-time. These sources can be combined in a bit-wise fashion. So, for
 * example, a <code>ACTIVE_PART | ACTIVE_CONTEXT</code> source includes change
 * to both the active context and the active part.
 * </p>
 * <p>
 * The values assigned to each source indicates its relative priority. The
 * higher the value, the more priority the source is given in resolving
 * conflicts. Another way to look at this is that the higher the value, the more
 * "local" the source is to what the user is currently doing. This is similar
 * to, but distinct from the concept of components. The nesting support provided
 * by components represent only one source (<code>ACTIVE_SITE</code>) that
 * the workbench understands.
 * </p>
 * <p>
 * Note that for backward compatibility, we must reserve the lowest three bits
 * for <code>Priority</code> instances using the old
 * <code>HandlerSubmission</code> mechanism. This mechanism was used in
 * Eclipse 3.0.
 * </p>
 * <p>
 * <b>Note in 3.3:</b>
 * </p>
 * <p>
 * Currently, source variables are not extensible by user plugins, and
 * the number of bits available for resolving conflicts is limited.  When
 * the variable sources become user extensible a new conflict resolution
 * mechanism will be implemented.
 * </p>
 * @noimplement This interface is not intended to be implemented by clients.
 * @noextend This interface is not intended to be extended by clients.
 *
 * @see org.opencherry.ui.ISourceProvider
 * @since 3.1
 */
struct CHERRY_UI ISources {

  /**
   * The priority given to default handlers and handlers that are active
   * across the entire workbench.
   */
  static const int WORKBENCH(); // = 0;

  /**
   * The priority given when the activation is defined by a handler submission
   * with a legacy priority.
   */
  static const int LEGACY_LEGACY(); // = 1;

  /**
   * The priority given when the activation is defined by a handler submission
   * with a low priority.
   */
  static const int LEGACY_LOW(); // = 1 << 1;

  /**
   * The priority given when the activation is defined by a handler submission
   * with a medium priority.
   */
  static const int LEGACY_MEDIUM(); // = 1 << 2;

  /**
   * The priority given when the source includes a particular context.
   */
  static const int ACTIVE_CONTEXT(); // = 1 << 6;

  /**
   * The variable name for the active contexts. This is for use with the
   * <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   * @since 3.2
   */
  static const std::string ACTIVE_CONTEXT_NAME(); // = "activeContexts"; //$NON-NLS-1$

  /**
   * The priority given when the source includes a particular action set.
   * @since 3.2
   */
  static const int ACTIVE_ACTION_SETS(); // = 1 << 8;

  /**
   * The variable name for the active action sets. This is for use with the
   * {@link ISourceProvider} and {@link IEvaluationContext}.
   * @since 3.2
   */
  static const std::string ACTIVE_ACTION_SETS_NAME(); // = "activeActionSets"(); //$NON-NLS-1$

  /**
   * The priority given when the source includes the currently active shell.
   */
  static const int ACTIVE_SHELL(); // = 1 << 10();

  /**
   * The variable name for the active shell. This is for use with the
   * <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   */
  static const std::string ACTIVE_SHELL_NAME(); // = "activeShell"(); //$NON-NLS-1$

  /**
   * The priority given when the source includes the currently active
   * workbench window shell.
   * @since 3.2
   */
  static const int ACTIVE_WORKBENCH_WINDOW_SHELL(); // = 1 << 12();

  /**
   * The variable name for the active workbench window shell. This is for use
   * with the <code>ISourceProvider</code> and
   * <code>IEvaluationContext</code>.
   * @since 3.2
   */
  static const std::string ACTIVE_WORKBENCH_WINDOW_SHELL_NAME(); // = "activeWorkbenchWindowShell"(); //$NON-NLS-1$

  /**
   * The priority given when the source includes the currently active
   * workbench window.
   */
  static const int ACTIVE_WORKBENCH_WINDOW(); // = 1 << 14();

  /**
   * The variable name for the active workbench window. This is for use with
   * the <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   */
  static const std::string ACTIVE_WORKBENCH_WINDOW_NAME(); // = "activeWorkbenchWindow"(); //$NON-NLS-1$

  /**
   * The priority given when the source includes subordinate properties of the currently active
   * workbench window.
   *
   * @since 3.3
   */
  static const int ACTIVE_WORKBENCH_WINDOW_SUBORDINATE(); // = 1 << 15();

  /**
   * The variable name for the coolbar visibility state of the active
   * workbench window. This is for use with the <code>ISourceProvider</code>
   * and <code>IEvaluationContext</code>.
   *
   * @since 3.3
   */
  static const std::string ACTIVE_WORKBENCH_WINDOW_IS_COOLBAR_VISIBLE_NAME(); // = ACTIVE_WORKBENCH_WINDOW_NAME
     // + ".isCoolbarVisible"(); //$NON-NLS-1$

  /**
   * The variable name for the perspective bar visibility state of the active
   * workbench window. This is for use with the <code>ISourceProvider</code>
   * and <code>IEvaluationContext</code>.
   *
   * @since 3.3
   */
  static const std::string ACTIVE_WORKBENCH_WINDOW_IS_PERSPECTIVEBAR_VISIBLE_NAME(); // = ACTIVE_WORKBENCH_WINDOW_NAME
     // + ".isPerspectiveBarVisible"(); //$NON-NLS-1$

  /**
   * The variable name for the current perspective of the active workbench
   * window. This is for use with the <code>ISourceProvider</code> and
   * <code>IEvaluationContext</code>.
   *
   * @since 3.4
   */
  static const std::string ACTIVE_WORKBENCH_WINDOW_ACTIVE_PERSPECTIVE_NAME(); // = ACTIVE_WORKBENCH_WINDOW_NAME
   //+ ".activePerspective"(); //$NON-NLS-1$

  /**
   * The priority given when the source includes the active editor part.
   */
  static const int ACTIVE_EDITOR(); // = 1 << 16();

  /**
   * The variable name for the active editor part. This is for use with the
   * <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   * @since 3.2
   */
  static const std::string ACTIVE_EDITOR_NAME(); // = "activeEditor"(); //$NON-NLS-1$

  /**
   * The priority given when the source includes the active editor identifier.
   *
   * @since 3.2
   */
  static const int ACTIVE_EDITOR_ID(); // = 1 << 18();

  /**
   * The variable name for the active editor identifier. This is for use with
   * the <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   *
   * @since 3.2
   */
  static const std::string ACTIVE_EDITOR_ID_NAME(); // = "activeEditorId"(); //$NON-NLS-1$

  /**
   * The priority given when the source includes the active part.
   */
  static const int ACTIVE_PART(); // = 1 << 20();

  /**
   * The variable name for the active part. This is for use with the
   * <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   */
  static const std::string ACTIVE_PART_NAME(); // = "activePart"(); //$NON-NLS-1$

  /**
   * The priority given when the source includes the active part id.
   *
   * @since 3.2
   */
  static const int ACTIVE_PART_ID(); // = 1 << 22();

  /**
   * The variable name for the active part id. This is for use with the
   * <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   *
   * @since 3.2
   */
  static const std::string ACTIVE_PART_ID_NAME(); // = "activePartId"(); //$NON-NLS-1$

  /**
   * The priority given when the source includes the active workbench site. In
   * the case of nesting components, one should be careful to only activate
   * the most nested component.
   */
  static const int ACTIVE_SITE(); // = 1 << 26();

  /**
   * The variable name for the active workbench site. This is for use with the
   * <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   */
  static const std::string ACTIVE_SITE_NAME(); // = "activeSite"(); //$NON-NLS-1$

  /**
   * The variable for the showIn selection.  This is for use with the
   * <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   * @since 3.4
   * @see IShowInSource
   */
  static const std::string SHOW_IN_SELECTION(); // = "showInSelection"(); //$NON-NLS-1$

  /**
   * The variable for the showIn input.  This is for use with the
   * <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   * @since 3.4
   * @see IShowInSource
   */
  static const std::string SHOW_IN_INPUT(); // = "showInInput"(); //$NON-NLS-1$

  /**
   * The priority given when the source includes the current selection.
   */
  static const int ACTIVE_CURRENT_SELECTION(); // = 1 << 30();

  /**
   * The variable name for the active selection. This is for use with the
   * <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   * @since 3.2
   */
  static const std::string ACTIVE_CURRENT_SELECTION_NAME(); // = "selection"(); //$NON-NLS-1$

  /**
   * The priority given when the source includes the current menu.
   * @since 3.2
   */
  static const int ACTIVE_MENU(); // = 1 << 31();

  /**
   * The variable name for the active menu. This is for use with the
   * {@link ISourceProvider} and {@link IEvaluationContext}.
   * @since 3.2
   */
  static const std::string ACTIVE_MENU_NAME(); // = "activeMenu"(); //$NON-NLS-1$

  /**
   * The variable name for the <b>local</b> selection, available while a
   * context menu is visible.
   *
   * @since 3.3
   */
  static const std::string ACTIVE_MENU_SELECTION_NAME(); // = "activeMenuSelection"();  //$NON-NLS-1$

  /**
   * The variable name for the <b>local</b> editor input which is sometimes
   * available while a context menu is visible.
   *
   * @since 3.3
   */
  static const std::string ACTIVE_MENU_EDITOR_INPUT_NAME(); // = "activeMenuEditorInput"();  //$NON-NLS-1$

  /**
   * The variable name for the active focus Control, when provided by the
   * IFocusService.
   *
   * @since 3.3
   */
  static const std::string ACTIVE_FOCUS_CONTROL_NAME(); // = "activeFocusControl"(); //$NON-NLS-1$

  /**
   * The variable name for the active focus Control id, when provided by the
   * IFocusService.
   *
   * @since 3.3
   */
  static const std::string ACTIVE_FOCUS_CONTROL_ID_NAME(); // = "activeFocusControlId"(); //$NON-NLS-1$
};

}

#endif /*CHERRYISOURCES_H_*/
