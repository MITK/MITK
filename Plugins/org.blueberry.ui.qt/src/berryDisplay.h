/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYDISPLAY_H_
#define BERRYDISPLAY_H_

#include <Poco/Runnable.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

class BERRY_UI_QT Display
{

public:

  virtual ~Display();

  static Display* GetDefault();

  /**
   * Returns true if the calling thread is the same thread
   * who created this Display instance.
   * @return
   */
  virtual bool InDisplayThread() = 0;

  virtual void AsyncExec(Poco::Runnable*) = 0;
  virtual void SyncExec(Poco::Runnable*) = 0;

  virtual int RunEventLoop() = 0;
  virtual void ExitEventLoop(int code) = 0;

protected:

  /**
   * This method must be called from within the UI thread
   * and should create the Display instance and initialize
   * variables holding implementation specific thread data.
   */
  virtual void CreateDisplay() = 0;

  static Display* instance;

};

}

#endif /* BERRYDISPLAY_H_ */
