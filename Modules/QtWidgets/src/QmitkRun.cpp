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
#include <QScopeGuard>
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
      // Nothing is filtered while a modal dialog is up. The operation may have
      // raised one itself and be waiting for the answer: reader and writer
      // options are asked that way, and swallowing the input leaves the user
      // unable to give it. Qt's own modality already keeps input away from
      // every other window meanwhile, so there is nothing left here to do.
      //
      // Deciding this from the receiver does not work. A press is delivered to
      // the QWindow before the widget inside it, and there the receiver is not
      // a QWidget at all, so a test for the dialog's own widgets never matches
      // and the event is gone before the widget is ever asked.
      if (nullptr != QApplication::activeModalWidget())
        return QObject::eventFilter(watched, event);

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
        // A drag that started outside the application delivers these without
        // a single mouse event of ours, so nothing above keeps it out. Left
        // through, a file dropped on the window during a load would start a
        // second one from inside the first.
        case QEvent::DragEnter:
        case QEvent::DragMove:
        case QEvent::Drop:
        // Closing the window quits every event loop on this thread, the nested
        // one below included, and then tears the workbench down: views are
        // destroyed and plugins stopped while the operation is still running on
        // its worker, holding the data storage and the dispatcher it needs.
        // Kept out until the operation is over, after which the user can close
        // the window as usual.
        case QEvent::Close:
        case QEvent::Quit:
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

    {
      // The worker writes into this frame through the reference capture above,
      // so leaving before it is done is a write to a frame that no longer
      // exists. Neither ~QFuture nor ~QFutureWatcher waits, so the wait belongs
      // on every path out of here, an exception thrown by the waiting below
      // included.
      //
      // The loop is also left without the task being done in the ordinary
      // course of events: QCoreApplication::exit() exits every loop on the
      // thread, nested ones included.
      const auto joinWorker = qScopeGuard([&future]()
        {
          // Waiting means pumping events, not QFuture::waitForFinished(): the
          // worker hands data storage mutations to this thread and blocks on
          // them, so a thread that stops answering deadlocks it.
          //
          // An event handler that throws must not carry the exception out of a
          // destructor running during unwinding, and must not cut the wait
          // short either, so it is swallowed and the wait resumed.
          while (!future.isFinished())
          {
            try
            {
              QmitkProcessEventsUntil([&future]() { return future.isFinished(); });
            }
            catch (...)
            {
            }
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
    }

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
