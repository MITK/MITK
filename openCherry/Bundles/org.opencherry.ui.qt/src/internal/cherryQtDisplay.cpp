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
    emit NewRunnable(runnable);
  }

  void QtDisplay::SyncExec(Poco::Runnable*)
  {

  }

  void QtDisplay::CreateDisplay()
  {
    Display::instance = this;
    displayThread = QThread::currentThread();

    connect(this, SIGNAL(NewRunnable(Poco::Runnable*)), this, SLOT(ExecuteRunnable(Poco::Runnable*)));
  }

  void QtDisplay::ExecuteRunnable(Poco::Runnable* runnable)
  {
    runnable->run();
  }

}
