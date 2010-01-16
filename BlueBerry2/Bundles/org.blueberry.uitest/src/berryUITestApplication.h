/*=========================================================================
 
 Program:   BlueBerry Platform
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

#ifndef BERRYUITESTAPPLICATION_H_
#define BERRYUITESTAPPLICATION_H_

#include <berryIApplication.h>
#include <berryITestHarness.h>
#include <berryTestableObject.h>
#include <berryPlatformException.h>

#include "berryUITestDll.h"

namespace berry
{

class BERRY_UITEST_EXPORT UITestApplication: public IApplication, public ITestHarness
{

public:

  int Start();

  void Stop();

  /*
   * @see berry#ITestHarness#RunTests()
   */
  void RunTests();

private:

  TestableObject::Pointer testableObject;
  int testDriverResult;
  std::string testPlugin;

  /*
   * return the application to run, or null if not even the default application
   * is found.
   */
  IApplication* GetApplication() throw (CoreException);

  /**
   * The -BlueBerry.testApplication argument specifies the application to be run.
   * If the argument is not set, an empty string is returned and the UITestApplication
   * itself will be started.
   */
  std::string GetApplicationToRun();

  int RunApplication(IApplication* application);

  int RunUITestWorkbench();

  struct TestRunnable: public Poco::Runnable
  {
    TestRunnable(UITestApplication* app, const std::string& testPlugin);

    void run();

  private:
    UITestApplication* app;
    std::string testPlugin;
  };
};

}

#endif /* BERRYUITESTAPPLICATION_H_ */
