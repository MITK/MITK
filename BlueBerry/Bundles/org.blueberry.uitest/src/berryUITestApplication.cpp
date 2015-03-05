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

#include "berryUITestApplication.h"

#include <berryBlueBerryTestDriver.h>
#include <berryPlatformUI.h>
#include <berryIExtensionRegistry.h>
#include <berryIExtension.h>
#include <berryStarter.h>

#include "internal/berryUITestWorkbenchAdvisor.h"

namespace berry
{

class WorkbenchCloseRunnable: public Poco::Runnable
{
public:
  WorkbenchCloseRunnable(IWorkbench* workbench) :
    workbench(workbench)
  {
  }

  void run()
  {
    workbench->Close();
  }

private:
  IWorkbench* workbench;
};

UITestApplication::TestRunnable::TestRunnable(UITestApplication* app,
    const QString& testPlugin) :
  app(app), testPlugin(testPlugin)
{
}

void UITestApplication::TestRunnable::run()
{
  try
  {
    app->testDriverResult = BlueBerryTestDriver::Run(testPlugin, true);
  }
  catch (const ctkException& e)
  {
    qWarning() << e.printStackTrace();
  }
  catch (const std::exception& e)
  {
    qWarning() << e.what();
  }
}

UITestApplication::UITestApplication()
{

}

int UITestApplication::Start()
{
  // Get the plug-in to test
  try
  {
    testPlugin = QString::fromStdString(Platform::GetConfiguration().getString(
                                          Platform::ARG_TESTPLUGIN.toStdString()));
  } catch (const Poco::NotFoundException& /*e*/)
  {
    BERRY_ERROR << "You must specify a test plug-in id via "
        << Platform::ARG_TESTPLUGIN << "=<id>";
    return 1;
  }

  // Get the application to test
  IApplication* application = GetApplication();
  poco_assert(application);
  int result = RunApplication(application);

  if (IApplication::EXIT_OK != result)
  {
    std::cerr << "UITestRunner: Unexpected result from running application " << application << ": " << result << std::endl;
  }
  return testDriverResult;
}

void UITestApplication::Stop()
{
  IWorkbench* workbench = PlatformUI::GetWorkbench();
  if (!workbench)
  return;
  Display* display = workbench->GetDisplay();
  WorkbenchCloseRunnable runnable(workbench);
  display->SyncExec(&runnable);
}

void UITestApplication::RunTests()
{
  TestRunnable runnable(this, testPlugin);
  testableObject->TestingStarting();
  testableObject->RunTest(&runnable);
  testableObject->TestingFinished();
}

IApplication* UITestApplication::GetApplication()
{

  IExtension::Pointer extension;
  /*Platform::GetExtensionPointService()->GetExtension(
      Starter::XP_APPLICATIONS, GetApplicationToRun());*/

  QList<IConfigurationElement::Pointer> extensions(
        Platform::GetExtensionRegistry()->GetConfigurationElementsFor(Starter::XP_APPLICATIONS));

  QString appToRun = GetApplicationToRun();
  QString id;
  foreach (const IConfigurationElement::Pointer& configElem, extensions)
  {
    id = configElem->GetAttribute("id");
    if(id == appToRun)
    {
      extension = configElem->GetDeclaringExtension();
      break;
    }
  }

  IApplication* app = 0;

  if (extension)
  {
    QList<IConfigurationElement::Pointer> elements(
          extension->GetConfigurationElements());
    if (!elements.isEmpty())
    {
      QList<IConfigurationElement::Pointer> runs(
            elements[0]->GetChildren("run"));
      if (!runs.isEmpty())
      {
        app = runs[0]->CreateExecutableExtension<IApplication> ("class");
      }
    }
    return app;
  }

  return this;
}

QString UITestApplication::GetApplicationToRun()
{

  QString testApp;
  try
  {
    testApp = QString::fromStdString(Platform::GetConfiguration().getString(
                                       Platform::ARG_TESTAPPLICATION.toStdString()));
  }
  catch (const Poco::NotFoundException&)
  {
  }

  return testApp;
}

int UITestApplication::RunApplication(IApplication* application)
{
  testableObject = PlatformUI::GetTestableObject();
  testableObject->SetTestHarness(ITestHarness::Pointer(this));

  if (application == dynamic_cast<IApplication*>(this))
  return RunUITestWorkbench();

  return application->Start();
}

int UITestApplication::RunUITestWorkbench()
{
  Display* display = PlatformUI::CreateDisplay();
  UITestWorkbenchAdvisor advisor;
  return PlatformUI::CreateAndRunWorkbench(display, &advisor);
}

}
