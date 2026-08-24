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
#include <vector>

namespace mitk
{
  class BaseData;
}

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

/** \brief Build the VTK representation of data before a worker reads it.
 *
 * mitk::Image and mitk::Surface build their VTK representation on first
 * access, so a worker asking for it would build it there while the mappers on
 * the thread that owns the data read the very same object.
 * mitk::MultiLabelSegmentation is covered through the group images that both
 * of them actually read.
 *
 * Prefer declaring the data through QmitkRunWithInputBlocked(), which calls
 * this. Call it directly only where the work is not run through that helper.
 *
 * Cheap for anything already on display, since the mappers have built it
 * already. Does nothing for data that has no VTK representation.
 *
 * \param[in] data The data about to be handed to a worker. May be nullptr.
 */
MITKQTWIDGETS_EXPORT void QmitkPrebuildVtkRepresentation(const mitk::BaseData *data);

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
 *
 * \param[in] task The operation to run on a worker.
 * \param[in] read Data on display that the task reads. Its VTK representation
 *        is built here, on this thread, so that the task does not build it on
 *        the worker while the mappers are reading the same object. Declare
 *        everything the task touches; what it creates itself needs no entry.
 */
MITKQTWIDGETS_EXPORT void QmitkRunWithInputBlocked(std::function<void()> task,
                                                   const std::vector<const mitk::BaseData *> &read = {});

#endif
