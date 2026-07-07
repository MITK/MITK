/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Copied verbatim from Modules/QtWidgets/test.

#ifndef QmitkTestQApplication_h
#define QmitkTestQApplication_h

#include <mitkRenderingTestHelper.h>

#include <QApplication>

#include <string>
#include <vector>

extern std::vector<std::string> globalCmdLineArgs;

/**
 * Returns the process-wide QApplication, creating it on first use.
 *
 * A single, never-destroyed QApplication is required for QtWidgets tests
 * because:
 *   1. NSApplication on macOS is a process singleton; tearing down
 *      QApplication and recreating it across tests has been observed to
 *      SegFault on macOS Tahoe (Qt 6.10).
 *   2. Qt-bound process-global state (e.g. an observer registered by
 *      QmitkMxNMultiWidget on the mitk::RenderingManager singleton, or
 *      interaction state machines loaded from XML) outlives any single
 *      QApplication; a second-cycle QApplication would inherit dangling
 *      Qt-bound state from the first.
 *   3. QApplication stores 'argc' by reference and 'argv' as a pointer;
 *      both must outlive the QApplication, so their backing storage is
 *      held in function-local statics here rather than in the test
 *      fixture's setUp().
 *
 * Test fixtures call this from setUp() and do not delete the returned
 * instance. An exit-time "QThreadStorage: entry N destroyed before end of
 * thread" debug log may appear; this is harmless Qt behaviour for the
 * deliberately leaked QApplication and not a regression.
 */
inline QApplication& EnsureQApplication()
{
  if (auto* const existing = QApplication::instance())
  {
    return *static_cast<QApplication*>(existing);
  }

  static mitk::RenderingTestHelper::ArgcHelperClass s_CmdLineArgs(globalCmdLineArgs);
  static int s_Argc = s_CmdLineArgs.GetArgc();
  static char** s_Argv = s_CmdLineArgs.GetArgv();
  static auto* const s_App = new QApplication(s_Argc, s_Argv);
  return *s_App;
}

#endif
