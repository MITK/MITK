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

#ifndef BERRYUITESTAPPLICATION_H_
#define BERRYUITESTAPPLICATION_H_

#include <berryIApplication.h>
#include <berryITestHarness.h>
#include <berryTestableObject.h>
#include <berryPlatformException.h>

#include <org_blueberry_uitest_Export.h>

namespace berry
{

class BERRY_UITEST_EXPORT UITestApplication: public QObject, public IApplication, public ITestHarness
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication berry::ITestHarness)

public:

  UITestApplication();

  int Start();

  void Stop();

  /*
   * @see berry#ITestHarness#RunTests()
   */
  void RunTests();

private:

  TestableObject::Pointer testableObject;
  int testDriverResult;
  QString testPlugin;

  /*
   * return the application to run, or null if not even the default application
   * is found.
   */
  IApplication* GetApplication();

  /**
   * The -BlueBerry.testApplication argument specifies the application to be run.
   * If the argument is not set, an empty string is returned and the UITestApplication
   * itself will be started.
   */
  QString GetApplicationToRun();

  int RunApplication(IApplication* application);

  int RunUITestWorkbench();

  struct TestRunnable: public Poco::Runnable
  {
    TestRunnable(UITestApplication* app, const QString& testPlugin);

    void run();

  private:
    UITestApplication* app;
    QString testPlugin;
  };
};

}

#endif /* BERRYUITESTAPPLICATION_H_ */
