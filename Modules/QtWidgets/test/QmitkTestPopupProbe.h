/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTestPopupProbe_h
#define QmitkTestPopupProbe_h

#include <QApplication>
#include <QElapsedTimer>
#include <QMenu>
#include <QObject>
#include <QTimer>
#include <QWidget>

#include <functional>

/**
 * Counts and closes every popup that opens while the probe lives.
 *
 * A menu runs its own event loop, so a test that triggers one would block
 * until somebody closes it. The probe polls from a repeating timer, which
 * fires inside that nested loop as well as in the test's own pumping, so it
 * must be created before the first event that could open a popup: a menu
 * opened synchronously by that event is then still closed and counted.
 */
class QmitkTestPopupProbe
{
public:
  /** Called with each popup before it is closed, e.g. to read a menu's entries. */
  std::function<void(QWidget*)> inspect;

  QmitkTestPopupProbe()
  {
    m_Timer.setInterval(5);
    QObject::connect(&m_Timer, &QTimer::timeout, &m_Timer, [this]()
    {
      auto* popup = QApplication::activePopupWidget();
      if (nullptr == popup)
      {
        return;
      }
      ++m_Count;
      if (this->inspect)
      {
        this->inspect(popup);
      }
      popup->close();
    });
    m_Timer.start();
  }

  ~QmitkTestPopupProbe()
  {
    m_Timer.stop();
  }

  /** The number of popups that opened so far. */
  int Count() const
  {
    return m_Count;
  }

  /** Run the event loop for 'milliseconds', long enough for a deferred popup
   *  to open and for the probe to close it. */
  static void Pump(int milliseconds = 100)
  {
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < milliseconds)
    {
      QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }
  }

private:
  QTimer m_Timer;
  int m_Count = 0;
};

#endif
