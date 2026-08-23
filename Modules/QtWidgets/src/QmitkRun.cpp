/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkRun.h>

#include <QApplication>
#include <QCoreApplication>
#include <QEventLoop>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QThread>
#include <QWidget>

#include <QtConcurrent>

#include <exception>

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
      // A dialog the operation raised itself has to stay usable, or the
      // operation waits forever for an answer the user cannot give. Reader and
      // writer options are asked this way.
      if (auto* modal = QApplication::activeModalWidget(); nullptr != modal)
      {
        auto* widget = qobject_cast<QWidget*>(watched);

        if (nullptr != widget && (widget == modal || modal->isAncestorOf(widget)))
          return QObject::eventFilter(watched, event);
      }

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

  /**
   * Runs the task on a worker thread and waits for it while this thread keeps
   * processing events, so that whatever the task reports through
   * mitk::ProgressTask is drawn and the data storage can still hand it work.
   * Rethrows here whatever the task threw there.
   */
  void RunAndWait(const std::function<void()>& task, const std::function<void()>& onFinished)
  {
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

    QObject::connect(&watcher, &QFutureWatcher<void>::finished, &loop, [&]() {
      if (onFinished)
        onFinished();

      loop.quit();
    });

    watcher.setFuture(future);
    loop.exec();

    // The loop can be left without the task being done: QCoreApplication::exit(),
    // which closing the window triggers, exits every loop on the thread, nested
    // ones included. See QmitkProcessEventsUntil().
    QmitkProcessEventsUntil([&future]() { return future.isFinished(); });

    if (exception)
      std::rethrow_exception(exception);
  }
}

void QmitkProcessEventsUntil(const std::function<bool()>& done)
{
  while (!done())
  {
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    // Not a bare spin: this only ever runs while something has gone wrong
    // enough to leave the loop early, and burning a core through it would
    // slow down the very task being waited for.
    QThread::msleep(10);
  }
}

void QmitkRunAsyncBlocking(const QString& title, const QString& label, std::function<void()> task)
{
  QProgressDialog dialog(label, {}, 0, 0);
  dialog.setWindowModality(Qt::ApplicationModal);
  dialog.setWindowTitle(title);
  dialog.setMinimumDuration(250);
  dialog.show();

  RunAndWait(task, [&dialog]() { dialog.close(); });
}

void QmitkRunWithInputBlocked(std::function<void()> task)
{
  // No dialog: the progress notifications are the feedback.
  InputBlocker blocker;

  RunAndWait(task, nullptr);
}
