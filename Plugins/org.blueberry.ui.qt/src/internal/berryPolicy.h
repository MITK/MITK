/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYPOLICY_H
#define BERRYPOLICY_H

class QString;

namespace berry {
/**
 * A common facility for parsing the <code>org.eclipse.ui/.options</code>
 * file.
 */
class Policy
{
public:

  static const bool DEFAULT;

  static bool DEBUG_UI_GLOBAL();

  static bool DEBUG_DRAG_DROP();

  static bool DEBUG_PERSPECTIVES();

  /**
   * Flag to log stale jobs
   */
  static bool DEBUG_STALE_JOBS();

  /**
   * Whether to report all events entering through the common event framework
   * used by the commands architecture.
   *
   * @see ISourceProvider
   */
  static bool DEBUG_SOURCES();

  /**
   * Whether to print information about key bindings that are successfully
   * recognized within the system (as the keys are pressed).
   */
  static bool DEBUG_KEY_BINDINGS();

  /**
   * Whether to print information about every key seen by the system.
   */
  static bool DEBUG_KEY_BINDINGS_VERBOSE();

  /**
   * Whether to print debugging information about the execution of commands
   */
  static bool DEBUG_COMMANDS();

  /**
   * Whether to print debugging information about the internal state of the
   * context support within the workbench.
   */
  static bool DEBUG_CONTEXTS();

  /**
   * Whether to print debugging information about the performance of context
   * computations.
   */
  static bool DEBUG_CONTEXTS_PERFORMANCE();

  /**
   * Whether to print even more debugging information about the internal state
   * of the context support within the workbench.
   */
  static bool DEBUG_CONTEXTS_VERBOSE();

  /**
   * Whether to print debugging information about the internal state of the
   * command support (in relation to handlers) within the workbench.
   */
  static bool DEBUG_HANDLERS();

  /**
   * Whether to print debugging information about the performance of handler
   * computations.
   */
  static bool DEBUG_HANDLERS_PERFORMANCE();

  /**
   * Whether to print out verbose information about changing handlers in the
   * workbench.
   */
  static bool DEBUG_HANDLERS_VERBOSE();

  /**
   * Whether to print debugging information about unexpected occurrences and
   * important state changes in the operation history.
   */
  static bool DEBUG_OPERATIONS();

  /**
   * Whether to print out verbose information about the operation histories,
   * including all notifications sent.
   */
  static bool DEBUG_OPERATIONS_VERBOSE();

  /**
   * Whether or not to show system jobs at all times.
   */
  static bool DEBUG_SHOW_ALL_JOBS();

  /**
   * Whether or not to print contribution-related issues.
   */
  static bool DEBUG_CONTRIBUTIONS();

  /**
   * Which command identifier to print handler information for.  This
   * restricts the debugging output, so a developer can focus on one command
   * at a time.
   */
  static QString DEBUG_HANDLERS_VERBOSE_COMMAND_ID();

  /**
   * Whether or not additional working set logging will occur.
   */
  static bool DEBUG_WORKING_SETS();

private:

  static bool GetDebugOption(const QString& option);
};

}

#endif // BERRYPOLICY_H
