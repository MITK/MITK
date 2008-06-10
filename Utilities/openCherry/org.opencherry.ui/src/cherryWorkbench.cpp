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

#include "cherryWorkbench.h"

#include "internal/cherryViewRegistry.h"
#include "internal/cherryEditorRegistry.h"

#include "internal/cherryWorkbenchPlugin.h"

#include <Poco/Bugcheck.h>

namespace cherry
{

const std::string Workbench::DIALOG_ID_SHOW_VIEW =
    "org.opencherry.ui.dialogs.showview";

Workbench::Workbench()
{

}

Workbench::~Workbench()
{

}

void Workbench::InternalInit(WorkbenchAdvisor* advisor)
{
  poco_check_ptr(advisor);

  this->advisor = advisor;
}

bool Workbench::Init()
{
  bool bail = false;
  
  // create workbench window manager
  //windowManager = new WindowManager();

  // TODO Correctly order service initialization
  // there needs to be some serious consideration given to
  // the services, and hooking them up in the correct order
  //final EvaluationService restrictionService = new EvaluationService();
  //final EvaluationService evaluationService = new EvaluationService();

  //    StartupThreading.runWithoutExceptions(new StartupRunnable() {
  //
  //      public void runWithException() {
  //        serviceLocator.registerService(IRestrictionService.class,
  //            restrictionService);
  //        serviceLocator.registerService(IEvaluationService.class,
  //            evaluationService);
  //      }
  //    });

  // Initialize the activity support.
  //workbenchActivitySupport = new WorkbenchActivitySupport();
  //activityHelper = ActivityPersistanceHelper.getInstance();

  //    initializeDefaultServices();
  //    initializeFonts();
  //    initializeColors();
  //    initializeApplicationColors();

  // now that the workbench is sufficiently initialized, let the advisor
  // have a turn.

  advisor->InternalBasicInitialize(this->GetWorkbenchConfigurer());

   // attempt to restore a previous workbench state

  advisor->PreStartup();

  if (!advisor->OpenWindows())
  {
    bail = true;
  }

  if (bail)
    return false;

  //forceOpenPerspective();

  return true;
}

WorkbenchConfigurer::Pointer Workbench::GetWorkbenchConfigurer() 
{
  if (workbenchConfigurer.IsNull()) {
    workbenchConfigurer = new WorkbenchConfigurer();
  }
  return workbenchConfigurer;
}

WorkbenchAdvisor* Workbench::GetAdvisor() 
{
  return advisor;
}

IViewRegistry* Workbench::GetViewRegistry()
{
  return WorkbenchPlugin::GetDefault()->GetViewRegistry();
}

IEditorRegistry* Workbench::GetEditorRegistry()
{
  return WorkbenchPlugin::GetDefault()->GetEditorRegistry();
}

void Workbench::ShowPerspective(const std::string& /*perspectiveId*/)
{

}

}
