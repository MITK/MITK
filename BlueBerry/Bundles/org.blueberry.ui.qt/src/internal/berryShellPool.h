/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYSHELLPOOL_H_
#define BERRYSHELLPOOL_H_

#include "berryShell.h"

#include <list>

namespace berry
{

/**
 * Manages a pool of shells. This can be used instead of creating and destroying
 * shells. By reusing shells, they will never be disposed until the pool goes away.
 * This is useful in situations where client code may have cached pointers to the
 * shells to use as a parent for dialogs. It also works around bug 86226 (SWT menus
 * cannot be reparented).
 *
 * @since 3.1
 */
class ShellPool: public Object, public IShellListener
{

  int flags;

  /**
   * Parent shell (or null if none)
   *
   * The parentShell is owned by the WorkbenchWindow, which also owns a ShellPool instance
   */
  Shell::WeakPtr parentShell;

  QList<Shell::Pointer> availableShells;

  static const QString CLOSE_LISTENER; // = "close listener";

  //    DisposeListener disposeListener = new DisposeListener() {
  //        public void widgetDisposed(DisposeEvent e) {
  //            WorkbenchPlugin.log(new RuntimeException("Widget disposed too early!")); //$NON-NLS-1$
  //        }
  //    };

  friend class WorkbenchWindow;

public:

  berryObjectMacro(ShellPool)

  void ShellClosed(const ShellEvent::Pointer& e);

  /**
   * Creates a shell pool that allocates shells that are children of the
   * given parent and are created with the given flags.
   *
   * @param parentShell parent shell (may be null, indicating that this pool creates
   * top-level shells)
   * @param childFlags flags for all child shells
   */
  ShellPool(Shell::Pointer parentShell, int childFlags);

  /**
   * Returns a new shell. The shell must not be disposed directly, but it may be closed.
   * Once the shell is closed, it will be returned to the shell pool. Note: callers must
   * remove all listeners from the shell before closing it.
   */
  Shell::Pointer AllocateShell(IShellListener* closeListener);

};

}

#endif /* BERRYSHELLPOOL_H_ */
