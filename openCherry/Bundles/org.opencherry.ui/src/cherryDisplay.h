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


#ifndef CHERRYDISPLAY_H_
#define CHERRYDISPLAY_H_

#include <Poco/Runnable.h>

namespace cherry {

class Display
{

public:

  static Display* GetDefault();

  /**
   * Returns true if the calling thread is the same thread
   * who created this Display instance.
   * @return
   */
  virtual bool InDisplayThread() = 0;

  virtual void AsyncExec(Poco::Runnable*) = 0;
  virtual void SyncExec(Poco::Runnable*) = 0;

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

#endif /* CHERRYDISPLAY_H_ */
