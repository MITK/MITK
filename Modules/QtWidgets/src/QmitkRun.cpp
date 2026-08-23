/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkRun.h>

#include <QApplication>

namespace
{
  /**
   * Discards user input while an operation runs. The event loop below has to
   * keep turning so that progress is drawn and timers fire, which also means
   * clicks would otherwise be delivered.
   */
  class InputBlocker : public QObject
  {
  public:
    InputBlocker()
    {
      QApplication::setOverrideCursor(Qt::BusyCursor);
      qApp->installEventFilter(this);
    }

    ~InputBlocker() override
    {
      qApp->removeEventFilter(this);
      QApplication::restoreOverrideCursor();
    }

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
      switch (event->type())
      {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::Wheel:
        case QEvent::ContextMenu:
        case QEvent::Shortcut:
        case QEvent::ShortcutOverride:
        case QEvent::TabletPress:
        case QEvent::TabletRelease:
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
        case QEvent::TouchEnd:
          return true;

        default:
          return QObject::eventFilter(watched, event);
      }
    }
  };
}

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

void QmitkRunWithInputBlocked(std::function<void()> task)
{
  InputBlocker blocker;

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

  QObject::connect(&watcher, &QFutureWatcher<void>::finished, &loop, &QEventLoop::quit);

  watcher.setFuture(future);
  loop.exec();

  if (exception)
    std::rethrow_exception(exception);
}
