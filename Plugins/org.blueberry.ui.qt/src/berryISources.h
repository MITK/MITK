/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYISOURCES_H_
#define BERRYISOURCES_H_

#include <string>
#include <org_blueberry_ui_qt_Export.h>

namespace berry {

/**
 * \ingroup org_blueberry_ui_qt
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
 * @see org.blueberry.ui.ISourceProvider
 */
struct BERRY_UI_QT ISources
{

  /**
   * The priority given to default handlers and handlers that are active
   * across the entire workbench.
   */
  static int WORKBENCH(); // = 0;

  /**
   * The priority given when the source includes a particular context.
   */
  static int ACTIVE_CONTEXT(); // = 1 << 3;

  /**
   * The variable name for the active contexts. This is for use with the
   * <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   */
  static const QString ACTIVE_CONTEXT_NAME();

  /**
   * The priority given when the source includes a particular action set.
   */
  static int ACTIVE_ACTION_SETS(); // = 1 << 5;

  /**
   * The variable name for the active action sets. This is for use with the
   * {@link ISourceProvider} and {@link IEvaluationContext}.
   */
  static const QString ACTIVE_ACTION_SETS_NAME();

  /**
   * The priority given when the source includes the currently active shell.
   */
  static int ACTIVE_SHELL(); // = 1 << 7();

  /**
   * The variable name for the active shell. This is for use with the
   * <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   */
  static const QString ACTIVE_SHELL_NAME();

  /**
   * The priority given when the source includes the currently active
   * workbench window shell.
   */
  static int ACTIVE_WORKBENCH_WINDOW_SHELL(); // = 1 << 9();

  /**
   * The variable name for the active workbench window shell. This is for use
   * with the <code>ISourceProvider</code> and
   * <code>IEvaluationContext</code>.
   */
  static const QString ACTIVE_WORKBENCH_WINDOW_SHELL_NAME();

  /**
   * The priority given when the source includes the currently active
   * workbench window.
   */
  static int ACTIVE_WORKBENCH_WINDOW(); // = 1 << 11();

  /**
   * The variable name for the active workbench window. This is for use with
   * the <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   */
  static const QString ACTIVE_WORKBENCH_WINDOW_NAME();

  /**
   * The priority given when the source includes subordinate properties of the currently active
   * workbench window.
   */
  static int ACTIVE_WORKBENCH_WINDOW_SUBORDINATE(); // = 1 << 12();

  /**
   * The variable name for the coolbar visibility state of the active
   * workbench window. This is for use with the <code>ISourceProvider</code>
   * and <code>IEvaluationContext</code>.
   */
  static const QString ACTIVE_WORKBENCH_WINDOW_IS_TOOLBAR_VISIBLE_NAME();

  /**
   * The variable name for the perspective bar visibility state of the active
   * workbench window. This is for use with the <code>ISourceProvider</code>
   * and <code>IEvaluationContext</code>.
   */
  static const QString ACTIVE_WORKBENCH_WINDOW_IS_PERSPECTIVEBAR_VISIBLE_NAME();

  /**
   * The variable name for the status line visibility state of the active
   * workbench window. This is for use with the <code>ISourceProvider</code>
   * and <code>IEvaluationContext</code>.
   */
  static const QString ACTIVE_WORKBENCH_WINDOW_IS_STATUS_LINE_VISIBLE_NAME();

  /**
   * The variable name for the current perspective of the active workbench
   * window. This is for use with the <code>ISourceProvider</code> and
   * <code>IEvaluationContext</code>.
   */
  static const QString ACTIVE_WORKBENCH_WINDOW_ACTIVE_PERSPECTIVE_NAME();

  /**
   * The priority given when the source includes the active editor part.
   */
  static int ACTIVE_EDITOR(); // = 1 << 13();

  /**
   * The variable name for the active editor part. This is for use with the
   * <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   */
  static const QString ACTIVE_EDITOR_NAME();

  /**
   * The editor input of the currently active editor.
   */
  static const QString ACTIVE_EDITOR_INPUT_NAME();

  /**
   * The priority given when the source includes the active editor identifier.
   */
  static int ACTIVE_EDITOR_ID(); // = 1 << 15();

  /**
   * The variable name for the active editor identifier. This is for use with
   * the <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   */
  static const QString ACTIVE_EDITOR_ID_NAME();

  /**
   * The priority given when the source includes the active part.
   */
  static int ACTIVE_PART(); // = 1 << 17();

  /**
   * The variable name for the active part. This is for use with the
   * <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   */
  static const QString ACTIVE_PART_NAME();

  /**
   * The priority given when the source includes the active part id.
   */
  static int ACTIVE_PART_ID(); // = 1 << 19();

  /**
   * The variable name for the active part id. This is for use with the
   * <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   */
  static const QString ACTIVE_PART_ID_NAME();

  /**
   * The priority given when the source includes the active workbench site. In
   * the case of nesting components, one should be careful to only activate
   * the most nested component.
   */
  static int ACTIVE_SITE(); // = 1 << 23();

  /**
   * The variable name for the active workbench site. This is for use with the
   * <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   */
  static const QString ACTIVE_SITE_NAME();

  /**
   * The variable for the showIn selection.  This is for use with the
   * <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   * @see IShowInSource
   */
  static const QString SHOW_IN_SELECTION();

  /**
   * The variable for the showIn input.  This is for use with the
   * <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   * @see IShowInSource
   */
  static const QString SHOW_IN_INPUT();

  /**
   * The priority given when the source includes the current selection.
   */
  static int ACTIVE_CURRENT_SELECTION(); // = 1 << 27();

  /**
   * The variable name for the active selection. This is for use with the
   * <code>ISourceProvider</code> and <code>IEvaluationContext</code>.
   */
  static const QString ACTIVE_CURRENT_SELECTION_NAME();

  /**
   * The priority given when the source includes the current menu.
   */
  static int ACTIVE_MENU(); // = 1 << 30();

  /**
   * The variable name for the active menu. This is for use with the
   * {@link ISourceProvider} and {@link IEvaluationContext}.
   */
  static const QString ACTIVE_MENU_NAME();

  /**
   * The variable name for the <b>local</b> selection, available while a
   * context menu is visible.
   */
  static const QString ACTIVE_MENU_SELECTION_NAME();

  /**
   * The variable name for the <b>local</b> editor input which is sometimes
   * available while a context menu is visible.
   */
  static const QString ACTIVE_MENU_EDITOR_INPUT_NAME();

  /**
   * The variable name for the active focus Control, when provided by the
   * IFocusService.
   */
  static const QString ACTIVE_FOCUS_CONTROL_NAME();

  /**
   * The variable name for the active focus Control id, when provided by the
   * IFocusService.
   */
  static const QString ACTIVE_FOCUS_CONTROL_ID_NAME();
};

}

#endif /*BERRYISOURCES_H_*/
