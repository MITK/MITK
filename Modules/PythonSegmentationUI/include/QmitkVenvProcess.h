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

#include <string>

namespace mitk
{
  struct PostInstallStep;
}

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
   * \param executable The executable to run.
   * \param args Command-line arguments passed to \p executable.
   * \param finishedTimeoutMs Milliseconds to wait for completion; a negative
   *        value waits indefinitely. On start failure or timeout the process is
   *        killed and Result::success stays false.
   */
  MITKPYTHONSEGMENTATIONUI_EXPORT Result Run(const QString& executable, const QStringList& args, int finishedTimeoutMs = -1);

  /** \brief Outcome of RunStep(). */
  enum class StepResult
  {
    Succeeded,
    Failed,
    Cancelled
  };

  /** \brief Runs a post-install step of a virtual environment on its own.
   *
   * Runs the Python code of the step with the interpreter of the environment
   * and keeps the application responsive meanwhile. Once the step reports its
   * progress (see mitk::PostInstallStepProgress), a progress notification named
   * after the step shows it and offers to cancel, which stops the step. A step
   * that reports nothing, because it has nothing to do, ends without one.
   *
   * Runs a local event loop, during which the caller can be deleted.
   *
   * \param[in] venvName The virtual environment to run the step in.
   * \param[in] step The step. Whether it is optional does not matter here.
   * \param[out] output Receives what the step printed apart from its progress,
   *             for an error report. May be \c nullptr.
   */
  MITKPYTHONSEGMENTATIONUI_EXPORT StepResult RunStep(const std::string& venvName, const mitk::PostInstallStep& step, QString* output = nullptr);
}

#endif
