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

#include "SelectionServiceMitk.h"

// berry includes
#include <berryPlatformUI.h>
#include <berryQtWorkbenchAdvisor.h>

class SelectionServiceMITKWorkbenchAdvisor : public berry::QtWorkbenchAdvisor
{

public:

  static const std::string DEFAULT_PERSPECTIVE_ID;

  berry::WorkbenchWindowAdvisor* CreateWorkbenchWindowAdvisor(
      berry::IWorkbenchWindowConfigurer::Pointer configurer)
  {
    // Set an individual initial size
    configurer->SetInitialSize(berry::Point(600,400));
    // Set the window title
    configurer->SetTitle("MITK Selection Service");

    wwAdvisor.reset(new berry::WorkbenchWindowAdvisor(configurer));
    return wwAdvisor.data();
  }

  std::string GetInitialWindowPerspectiveId()
  {
    return DEFAULT_PERSPECTIVE_ID;
  }

private:

  QScopedPointer<berry::WorkbenchWindowAdvisor> wwAdvisor;

};

const std::string SelectionServiceMITKWorkbenchAdvisor::DEFAULT_PERSPECTIVE_ID = "org.mitk.example.extendedperspective";

SelectionServiceMitk::SelectionServiceMitk()
{
}

SelectionServiceMitk::~SelectionServiceMitk()
{
}

int SelectionServiceMitk::Start()
{
  berry::Display* display = berry::PlatformUI::CreateDisplay();
  wbAdvisor.reset(new SelectionServiceMITKWorkbenchAdvisor);

  int code = berry::PlatformUI::CreateAndRunWorkbench(display, wbAdvisor.data());

  // exit the application with an appropriate return code
  return code == berry::PlatformUI::RETURN_RESTART
              ? EXIT_RESTART : EXIT_OK;
}

void SelectionServiceMitk::Stop()
{

}
