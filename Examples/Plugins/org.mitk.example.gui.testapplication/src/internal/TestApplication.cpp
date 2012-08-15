/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "TestApplication.h"

#include <berryPlatformUI.h>
#include <berryQtWorkbenchAdvisor.h>
#include "TestApplicationWorkbenchWindowAdvisor.h"

class TestWorkbenchAdvisor : public berry::WorkbenchAdvisor
{

public:

  static const std::string DEFAULT_PERSPECTIVE_ID;

  berry::WorkbenchWindowAdvisor* CreateWorkbenchWindowAdvisor(
      berry::IWorkbenchWindowConfigurer::Pointer configurer)
  {
    //wwAdvisor.reset(new berry::WorkbenchWindowAdvisor(configurer));
    wwAdvisor.reset(new TestApplicationWorkbenchWindowAdvisor(configurer));
    return wwAdvisor.data();
  }

  /*void Initialize(berry::IWorkbenchConfigurer::Pointer configurer)
  {
    berry::WorkbenchAdvisor::Initialize(configurer);
    QString styleName = "teststyle";

    IQtStyleManager::Pointer styleManager = Platform::GetServiceRegistry().GetServiceById<IQtStyleManager>(IQtStyleManager::ID);
    styleManager->SetStyle(styleName);
  }*/

  std::string GetInitialWindowPerspectiveId()
  {
    return DEFAULT_PERSPECTIVE_ID;
  }

private:

  QScopedPointer<berry::WorkbenchWindowAdvisor> wwAdvisor;

};

const std::string TestWorkbenchAdvisor::DEFAULT_PERSPECTIVE_ID = "org.mitk.example.testperspective";

TestApplication::TestApplication()
{
}

TestApplication::~TestApplication()
{
}
 
int TestApplication::Start()
{
  berry::Display* display = berry::PlatformUI::CreateDisplay();

  wbAdvisor.reset(new TestWorkbenchAdvisor);
  int code = berry::PlatformUI::CreateAndRunWorkbench(display, wbAdvisor.data());
  
  // exit the application with an appropriate return code
  return code == berry::PlatformUI::RETURN_RESTART
              ? EXIT_RESTART : EXIT_OK;
}

void TestApplication::Stop()
{
  
}
