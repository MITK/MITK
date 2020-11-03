/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYQTDISPLAY_H_
#define BERRYQTDISPLAY_H_

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
