/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkVenvProcess_h
#define QmitkVenvProcess_h

#include <MitkPythonSegmentationUIExports.h>

#include <mitkFileSystem.h>

#include <QProcessEnvironment>
#include <QString>
#include <QStringList>

/**
  \brief Helpers for running an executable from a MITK-managed Python virtual
         environment as a subprocess.

  Centralises the one invariant every such call must honour: the child runs with
  PYTHONHOME / PYTHONPATH stripped, so it resolves its environment from the
  venv's own layout (pyvenv.cfg and site-packages) instead of MITK's embedded
  interpreter. Keeping that in a single place stops the historically duplicated
  copies of the pattern from drifting.
*/
namespace QmitkVenvProcess
{
  /** \brief The current process environment with PYTHONHOME and PYTHONPATH removed. */
  MITKPYTHONSEGMENTATIONUI_EXPORT QProcessEnvironment CleanEnvironment();

  /** \brief Lossless conversion of a filesystem path to a QString.
   *
   * fs::path::string() re-encodes to the native narrow code page on Windows,
   * which corrupts non-ASCII paths (e.g. a venv under a user profile with
   * accented characters) before they reach QProcess. Convert from the wide /
   * native representation instead.
   */
  MITKPYTHONSEGMENTATIONUI_EXPORT QString ToQString(const fs::path& path);

  struct Result
  {
    bool success = false;
    QString standardOutput;
    QString standardError;

    /** \brief Trimmed standard output and standard error concatenated. */
    QString combined() const { return (standardOutput + standardError).trimmed(); }
  };

  /** \brief Runs \p executable with \p args in a clean environment and blocks
   *         until it finishes.
   *
   * Blocks the calling thread, so run it from a worker thread (e.g. via
   * QmitkRunAsyncBlocking) rather than directly on the GUI thread.
   *
   * \param finishedTimeoutMs Milliseconds to wait for completion; a negative
   *        value waits indefinitely. On start failure or timeout the process is
   *        killed and Result::success stays false.
   */
  MITKPYTHONSEGMENTATIONUI_EXPORT Result Run(const QString& executable, const QStringList& args, int finishedTimeoutMs = -1);
}

#endif
