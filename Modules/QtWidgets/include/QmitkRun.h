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

#include <QEventLoop>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QString>
#include <QWidget>

#include <QtConcurrent>

#include <exception>
#include <functional>

/** \brief Runs a long task in a background thread while keeping the UI responsive.
 *
 * The function blocks until the task finishes, using a QEventLoop to process
 * events and prevent the UI from freezing.
 *
 * While the task is running, a QProgressDialog is shown with a delay of 250ms.
 *
 * Exceptions are caught in the background thread and rethrown in the calling thread.
 */
template<typename T>
T QmitkRunAsyncBlocking(const QString& title, const QString& label, std::function<T()> task)
{
  QProgressDialog dialog(label, {}, 0, 0);
  dialog.setWindowModality(Qt::ApplicationModal);
  dialog.setWindowTitle(title);
  dialog.setMinimumDuration(250);
  dialog.show();

  T result{};
  std::exception_ptr exception;

  auto future = QtConcurrent::run([&]() -> T {
    try
    {
      return task();
    }
    catch (...)
    {
      exception = std::current_exception();
      return T{};
    }
  });

  QFutureWatcher<T> watcher;
  QEventLoop loop;

  QObject::connect(&watcher, &QFutureWatcher<T>::finished, [&]() {
    result = watcher.result();
    dialog.close();
    loop.quit();
  });

  watcher.setFuture(future);
  loop.exec();

  if (exception)
    std::rethrow_exception(exception);

  return result;
}

MITKQTWIDGETS_EXPORT void QmitkRunAsyncBlocking(const QString& title, const QString& label, std::function<void()> task);

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
