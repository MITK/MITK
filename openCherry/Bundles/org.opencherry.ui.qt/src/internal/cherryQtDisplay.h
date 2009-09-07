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

#ifndef CHERRYQTDISPLAY_H_
#define CHERRYQTDISPLAY_H_

#include <QObject>
#include <QThread>

#include <cherryDisplay.h>

namespace cherry
{

class QtDisplay: public QObject, public Display
{

  Q_OBJECT

public:

  QtDisplay();

  bool InDisplayThread();

  void AsyncExec(Poco::Runnable*);
  void SyncExec(Poco::Runnable*);

  int RunEventLoop();
  void ExitEventLoop(int code);

signals:

void NewAsyncRunnable(Poco::Runnable*);
void NewSyncRunnable(Poco::Runnable*);

protected:

  /**
   * This method must be called from within the UI thread
   * and should create the Display instance and initialize
   * variables holding implementation specific thread data.
   */
  void CreateDisplay();

protected slots:

void ExecuteRunnable(Poco::Runnable*);

private:

  QThread * displayThread;

};

}

#endif /* CHERRYQTDISPLAY_H_ */
