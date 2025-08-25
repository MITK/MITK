/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkRun.h>

void QmitkRunAsyncBlocking(const QString& title, const QString& label, std::function<void()> task)
{
  QProgressDialog dialog(label, {}, 0, 0);
  dialog.setWindowModality(Qt::ApplicationModal);
  dialog.setWindowTitle(title);
  dialog.setMinimumDuration(250);
  dialog.show();

  std::exception_ptr exception;

  auto future = QtConcurrent::run([&]() {
    try
    {
      task();
    }
    catch (...)
    {
      exception = std::current_exception();
    }
  });

  QFutureWatcher<void> watcher;
  QEventLoop loop;

  QObject::connect(&watcher, &QFutureWatcher<void>::finished, [&]() {
    dialog.close();
    loop.quit();
  });

  watcher.setFuture(future);
  loop.exec();

  if (exception)
    std::rethrow_exception(exception);
}
