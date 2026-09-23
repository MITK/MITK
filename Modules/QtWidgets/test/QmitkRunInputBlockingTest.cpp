/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkProgressNotificationOverlay.h>
#include <QmitkRun.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QDialog>
#include <QEvent>
#include <QMouseEvent>
#include <QPushButton>
#include <QThread>
#include <QTimer>
#include <QWindow>

namespace
{
  /** \brief Counts the mouse presses the widget it watches is actually given. */
  class PressCounter : public QObject
  {
  public:
    int Count = 0;

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
      if (QEvent::MouseButtonPress == event->type())
        ++Count;

      return QObject::eventFilter(watched, event);
    }
  };

  struct ClickResult
  {
    bool HadWindow = false;
    int Presses = 0;
  };

  /**
   * Sends a press the way the platform does: to the window, which then works
   * out the widget inside it. Sending straight to the widget would take a path
   * that no application event filter ever sees the window on, and so would not
   * exercise what these tests are about.
   */
  void SendPressToWindow(QWindow *window, const QPoint &position)
  {
    const QPointF local(position);

    QMouseEvent press(QEvent::MouseButtonPress,
                      local,
                      local,
                      window->mapToGlobal(local),
                      Qt::LeftButton,
                      Qt::LeftButton,
                      Qt::NoModifier);

    QCoreApplication::sendEvent(window, &press);
  }
}

/**
 * QmitkRunWithInputBlocked() discards user input for as long as the operation
 * runs. These tests pin both halves of that: input has to stop reaching the
 * window the operation was started from, and it has to keep reaching a dialog
 * the operation raised itself, because the operation is blocked waiting for the
 * answer. Asking for reader or writer options works exactly that way.
 */
class QmitkRunInputBlockingTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkRunInputBlockingTestSuite);
  MITK_TEST(ModalDialogRaisedByTheOperation_StaysUsable_Success);
  MITK_TEST(WindowTheOperationRunsBehind_GetsNoInput_Success);
  MITK_TEST(OperationStartedWhileAnotherRuns_IsIgnored_Success);
  MITK_TEST(ProgressCardTheOperationPutsUp_StaysClickable_Success);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override { EnsureQApplication(); }

  void ModalDialogRaisedByTheOperation_StaysUsable_Success()
  {
    ClickResult result;

    // Fires from inside the event loop the helper below keeps turning, so the
    // input blocker is in place by the time the dialog goes up.
    QTimer::singleShot(0, qApp, [&result]() { result = ClickButtonInModalDialog(); });

    QmitkRunWithInputBlocked([]() { QThread::msleep(200); });

    CPPUNIT_ASSERT_MESSAGE("The dialog must have a window to send the press to", result.HadWindow);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A dialog the operation raised itself has to stay clickable",
                                 1,
                                 result.Presses);
  }

  void WindowTheOperationRunsBehind_GetsNoInput_Success()
  {
    QWidget window;
    window.resize(200, 100);

    QPushButton button(QStringLiteral("Do not press"), &window);
    button.setGeometry(20, 20, 120, 30);

    PressCounter counter;
    button.installEventFilter(&counter);

    window.show();

    ClickResult result;

    // Bound to the window, not to qApp: everything the callback touches lives
    // in this frame, and a call still pending once the frame is gone has to be
    // dropped rather than run against it.
    QTimer::singleShot(0, &window, [&]() {
      auto *handle = window.windowHandle();
      result.HadWindow = nullptr != handle;

      if (result.HadWindow)
        SendPressToWindow(handle, button.geometry().center());

      result.Presses = counter.Count;
    });

    QmitkRunWithInputBlocked([]() { QThread::msleep(200); });

    CPPUNIT_ASSERT_MESSAGE("The window must have a handle to send the press to", result.HadWindow);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Input must not reach the window the operation runs behind",
                                 0,
                                 result.Presses);
  }

  void OperationStartedWhileAnotherRuns_IsIgnored_Success()
  {
    auto attempted = false;
    auto nestedRan = false;

    // From the event loop the outer call keeps turning, which is where a user
    // event that got past the input blocker would arrive. The list of event
    // types the blocker knows about cannot be proven complete, so this is the
    // backstop for whatever it still misses.
    QTimer::singleShot(0, qApp, [&attempted, &nestedRan]()
      {
        attempted = true;
        QmitkRunWithInputBlocked([&nestedRan]() { nestedRan = true; });
      });

    QmitkRunWithInputBlocked([]() { QThread::msleep(200); });

    // Without this the test would pass just as well for a timer that never
    // fired, which is the wrong reason to be happy.
    CPPUNIT_ASSERT_MESSAGE("The second operation has to have been attempted", attempted);

    CPPUNIT_ASSERT_MESSAGE("A second operation must not start while one is running",
                           !nestedRan);
  }

  void ProgressCardTheOperationPutsUp_StaysClickable_Success()
  {
    QWidget window;
    window.resize(400, 300);

    // The card carries the cancel and the dismiss, which are the only controls
    // an operation leaves the user, so a press on it has to get through while
    // every other one is discarded.
    QmitkProgressNotificationOverlay overlay(&window);

    // Stands in for a notification card. Raising a real one would mean running
    // an operation for longer than the delay the overlay waits out first, and
    // then waiting on a timer for it to appear.
    QWidget card(&overlay);

    PressCounter counter;
    card.installEventFilter(&counter);

    window.show();
    overlay.show();

    // After both, because showing either puts the overlay where it belongs in
    // its parent, and an overlay with no cards in it has no height to hit.
    overlay.setGeometry(200, 200, 150, 60);
    card.setGeometry(0, 0, 150, 60);

    ClickResult result;

    QTimer::singleShot(0, &window, [&]() {
      auto *handle = window.windowHandle();
      result.HadWindow = nullptr != handle;

      if (result.HadWindow)
        SendPressToWindow(handle, overlay.geometry().center());

      result.Presses = counter.Count;
    });

    QmitkRunWithInputBlocked([]() { QThread::msleep(200); });

    CPPUNIT_ASSERT_MESSAGE("The window must have a handle to send the press to", result.HadWindow);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A press on the progress card has to reach it",
                                 1,
                                 result.Presses);
  }

private:
  static ClickResult ClickButtonInModalDialog()
  {
    ClickResult result;

    QDialog dialog;
    dialog.resize(200, 100);

    QPushButton button(QStringLiteral("OK"), &dialog);
    button.setGeometry(20, 20, 80, 30);

    PressCounter counter;
    button.installEventFilter(&counter);

    // Runs once the dialog's own event loop is up, so that it has a window.
    // Ends the dialog either way, so that a press that never arrives fails the
    // test rather than hanging it.
    QTimer::singleShot(0, &dialog, [&]() {
      auto *handle = dialog.windowHandle();
      result.HadWindow = nullptr != handle;

      if (result.HadWindow)
        SendPressToWindow(handle, button.geometry().center());

      result.Presses = counter.Count;
      dialog.accept();
    });

    dialog.exec();

    return result;
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkRunInputBlocking)
