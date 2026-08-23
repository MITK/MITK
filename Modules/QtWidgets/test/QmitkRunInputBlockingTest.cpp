/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

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

    QTimer::singleShot(0, qApp, [&]() {
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
