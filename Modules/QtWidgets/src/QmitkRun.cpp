/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkRun.h>

#include <QmitkProgressNotificationOverlay.h>

#include <mitkBaseData.h>
#include <mitkLog.h>

#include <QApplication>
#include <QFutureWatcher>
#include <QMouseEvent>
#include <QProgressDialog>
#include <QWindow>

#include <QtConcurrent>

#include <exception>

namespace
{
  /**
   * Whether a pointer event landed on a progress notification. The card
   * carries the cancel and the dismiss, which are the only controls an
   * operation leaves the user, so its clicks are the one kind worth letting
   * through.
   *
   * A press is delivered to the QWindow before the widget inside it, so the
   * receiver on its own does not answer this. It does say which window, and
   * inside one known window the rest is a plain hit test.
   */
  bool IsOverProgressNotification(QObject* watched, const QEvent* event)
  {
    const auto* pointerEvent = dynamic_cast<const QSinglePointEvent*>(event);

    if (nullptr == pointerEvent)
      return false;

    // Which top-level this is about is not guessed from the pointer position,
    // as QApplication::widgetAt() would: the event is being delivered to that
    // window right now, so it is the one thing here that is not in doubt. The
    // hit test then only has to look inside it.
    auto* widget = qobject_cast<QWidget*>(watched);

    if (nullptr == widget)
    {
      auto* window = qobject_cast<QWindow*>(watched);

      if (nullptr == window)
        return false;

      widget = QWidget::find(window->winId());

      if (nullptr == widget)
        return false;
    }

    widget = widget->window();

    auto* target = widget->childAt(widget->mapFromGlobal(pointerEvent->globalPosition().toPoint()));

    for (; nullptr != target; target = target->parentWidget())
    {
      if (nullptr != qobject_cast<QmitkProgressNotificationOverlay*>(target))
        return true;
    }

    return false;
  }

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
          // Only asked here, where the answer decides something, rather than
          // for every event that reaches the filter: it costs a hit test.
          return !IsOverProgressNotification(watched, event);

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

void QmitkRunWithInputBlocked(std::function<void()> task, const std::vector<const mitk::BaseData *> &read)
{
  // The blocker below keeps user input out, but the list of event types it
  // knows about cannot be proven complete and has grown by hand more than
  // once. This is the backstop for the ones still missing: an event that does
  // leak through costs an ignored click rather than a second operation started
  // into a reader that is not written to expect one.
  static thread_local bool running = false;

  if (running)
  {
    MITK_WARN << "Ignored an operation started while another one was already "
                 "running on this thread. Some user input reached the "
                 "application that should have been kept out.";
    return;
  }

  running = true;
  const auto done = qScopeGuard([]() { running = false; });

  // Before the worker starts, and on the thread that owns the data, which is
  // where a representation the mappers read has to be built.
  for (const auto *data : read)
    if (nullptr != data)
      data->PrebuildVtkRepresentation();

  // No dialog: the progress notifications are the feedback.
  InputBlocker blocker;

  RunAndWait(task, nullptr);
}
