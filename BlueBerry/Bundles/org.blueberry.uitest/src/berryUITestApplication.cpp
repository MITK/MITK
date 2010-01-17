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

#include "berryUITestApplication.h"

#include <berryBlueBerryTestDriver.h>
#include <berryPlatformUI.h>
#include <berryIExtensionPointService.h>
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
    const std::string& testPlugin) :
  app(app), testPlugin(testPlugin)
{
}

void UITestApplication::TestRunnable::run()
{
  try
  {
    app->testDriverResult = BlueBerryTestDriver::Run(testPlugin, true);
  } catch (Poco::IOException& e)
  {
    std::cerr << e.displayText() << std::endl;
    //TODO print stack trace
    //Debug::PrintStackTrace();
  }
}

int UITestApplication::Start()
{
  // Get the plug-in to test
  try
  {
    testPlugin = Platform::GetConfiguration().getString(
        Platform::ARG_TESTPLUGIN);
  } catch (const Poco::NotFoundException& e)
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

IApplication* UITestApplication::GetApplication() throw (CoreException)
{

  const IExtension* extension =
  Platform::GetExtensionPointService()->GetExtension(
      Starter::XP_APPLICATIONS, GetApplicationToRun());

  IApplication* app = 0;

  if (extension)
  {
    std::vector<IConfigurationElement::Pointer> elements(
        extension->GetConfigurationElements());
    if (elements.size() > 0)
    {
      std::vector<IConfigurationElement::Pointer> runs(
          elements[0]->GetChildren("run"));
      if (runs.size() > 0)
      {
        app = runs[0]->CreateExecutableExtension<IApplication> ("class"); //$NON-NLS-1$
      }
    }
    return app;
  }

  return this;
}

std::string UITestApplication::GetApplicationToRun()
{

  std::string testApp;
  try
  {
    testApp = Platform::GetConfiguration().getString(
        Platform::ARG_TESTAPPLICATION);
  }
  catch (Poco::NotFoundException)
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
