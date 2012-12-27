/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryQtDisplay.h"

#include "berryQtSafeApplication.h"

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

    QtSafeApplication::CreateInstance();

    this->connect(this, SIGNAL(NewAsyncRunnable(Poco::Runnable*)), this, SLOT(ExecuteRunnable(Poco::Runnable*)));
    this->connect(this, SIGNAL(NewSyncRunnable(Poco::Runnable*)), this, SLOT(ExecuteRunnable(Poco::Runnable*)), Qt::BlockingQueuedConnection);
  }

  void QtDisplay::ExecuteRunnable(Poco::Runnable* runnable)
  {
    runnable->run();
  }

}
