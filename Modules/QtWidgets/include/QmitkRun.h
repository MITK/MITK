/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRun_h
#define QmitkRun_h

#include <MitkQtWidgetsExports.h>

#include <QString>

#include <functional>

/** \brief Keeps the event loop turning until the given condition holds.
 *
 * For waiting out a task that this thread may itself be needed for:
 * mitk::StandaloneDataStorage hands its mutations to the thread that owns it,
 * so a wait that stops processing events deadlocks against its own worker.
 *
 * \param[in] done Polled until it returns true.
 */
MITKQTWIDGETS_EXPORT void QmitkProcessEventsUntil(const std::function<bool()>& done);

/** \brief Runs a long task in a background thread while keeping the UI responsive.
 *
 * The function blocks until the task finishes, but keeps processing events, so
 * that the UI does not freeze and the data storage can still hand the task the
 * part of its work that has to run on this thread.
 *
 * While the task is running, a QProgressDialog is shown with a delay of 250ms.
 *
 * Exceptions are caught in the background thread and rethrown in the calling thread.
 */
MITKQTWIDGETS_EXPORT void QmitkRunAsyncBlocking(const QString& title, const QString& label, std::function<void()> task);

/** \brief Runs a long task that produces a value. \sa QmitkRunAsyncBlocking()
 *
 * \return What the task returned. A task that threw returns nothing: its
 *         exception is rethrown here, as in the overload above.
 */
template<typename T>
T QmitkRunAsyncBlocking(const QString& title, const QString& label, std::function<T()> task)
{
  T result{};

  // Resolves to the overload above, which is the better match for an argument
  // that already is a std::function<void()>, so this does not recurse.
  QmitkRunAsyncBlocking(title, label, std::function<void()>([&result, &task]() { result = task(); }));

  return result;
}

/** \brief Runs a long task in a background thread and blocks user input.
 *
 * The function blocks until the task finishes, but keeps processing events,
 * so that anything the task reports through mitk::ProgressTask is shown and
 * keeps moving. No dialog is used: the progress notifications are the
 * feedback.
 *
 * User input is discarded for the duration. A second operation started into
 * a reader that is not written to expect one would be worse than an
 * unresponsive window.
 *
 * Exceptions are caught in the background thread and rethrown in the calling
 * thread.
 */
MITKQTWIDGETS_EXPORT void QmitkRunWithInputBlocked(std::function<void()> task);

#endif
