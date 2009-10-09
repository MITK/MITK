/*=========================================================================
 
 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "cherryQtDisplay.h"

#include <cherryPlatform.h>

#include <QApplication>

namespace cherry {

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

    char** argv;
    int& argc = Platform::GetRawApplicationArgs(argv);

    QApplication* app = new QApplication(argc, argv);

    this->connect(this, SIGNAL(NewAsyncRunnable(Poco::Runnable*)), this, SLOT(ExecuteRunnable(Poco::Runnable*)));
    this->connect(this, SIGNAL(NewSyncRunnable(Poco::Runnable*)), this, SLOT(ExecuteRunnable(Poco::Runnable*)), Qt::BlockingQueuedConnection);
  }

  void QtDisplay::ExecuteRunnable(Poco::Runnable* runnable)
  {
    runnable->run();
  }

}
