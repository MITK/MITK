/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQtDisplay.h"

#include <QApplication>

namespace berry {

QtDisplay::QtDisplay()
{
  this->CreateDisplay();
}

bool QtDisplay::InDisplayThread()
{
  return displayThread == QThread::currentThread();
}

  void QtDisplay::AsyncExec(Poco::Runnable* runnable)
  {
    emit NewAsyncRunnable(runnable);
  }

  void QtDisplay::SyncExec(Poco::Runnable* runnable)
  {
    emit NewSyncRunnable(runnable);
  }

  int QtDisplay::RunEventLoop()
  {
    return QApplication::exec();
  }

  void QtDisplay::ExitEventLoop(int code)
  {
    QApplication::exit(code);
  }

  void QtDisplay::CreateDisplay()
  {
    Display::instance = this;
    displayThread = QThread::currentThread();

    this->connect(this, SIGNAL(NewAsyncRunnable(Poco::Runnable*)), this, SLOT(ExecuteRunnable(Poco::Runnable*)));
    this->connect(this, SIGNAL(NewSyncRunnable(Poco::Runnable*)), this, SLOT(ExecuteRunnable(Poco::Runnable*)), Qt::BlockingQueuedConnection);
  }

  void QtDisplay::ExecuteRunnable(Poco::Runnable* runnable)
  {
    runnable->run();
  }

}
