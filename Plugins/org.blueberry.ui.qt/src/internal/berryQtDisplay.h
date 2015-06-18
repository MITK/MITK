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

#ifndef BERRYQTDISPLAY_H_
#define BERRYQTDISPLAY_H_

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include <berryDisplay.h>

#include <QObject>
#include <QThread>

namespace berry
{

class QtDisplay: public QObject, public Display
{

  Q_OBJECT

public:

  QtDisplay();

  bool InDisplayThread() override;

  void AsyncExec(Poco::Runnable*) override;
  void SyncExec(Poco::Runnable*) override;

  int RunEventLoop() override;
  void ExitEventLoop(int code) override;

signals:

void NewAsyncRunnable(Poco::Runnable*);
void NewSyncRunnable(Poco::Runnable*);

protected:

  /**
   * This method must be called from within the UI thread
   * and should create the Display instance and initialize
   * variables holding implementation specific thread data.
   */
  void CreateDisplay() override;

protected slots:

void ExecuteRunnable(Poco::Runnable*);

private:

  QThread * displayThread;

};

}

#endif /* BERRYQTDISPLAY_H_ */
