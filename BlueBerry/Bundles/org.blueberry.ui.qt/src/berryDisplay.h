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
