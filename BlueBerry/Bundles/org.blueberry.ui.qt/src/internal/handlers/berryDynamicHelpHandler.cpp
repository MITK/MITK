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

#include "berryDynamicHelpHandler.h"

#include "internal/berryWorkbenchPlugin.h"

#include <service/event/ctkEventAdmin.h>

namespace berry {

Object::Pointer DynamicHelpHandler::Execute(const SmartPointer<const ExecutionEvent>& /*event*/)
{
  ctkPluginContext* context = WorkbenchPlugin::GetDefault()->GetPluginContext();
  if (context == 0)
  {
    BERRY_WARN << "Plugin context not set, unable to open context help";
    return Object::Pointer();
  }

  ctkServiceReference eventAdminRef = context->getServiceReference<ctkEventAdmin>();
  ctkEventAdmin* eventAdmin = 0;
  if (eventAdminRef)
  {
    eventAdmin = context->getService<ctkEventAdmin>(eventAdminRef);
  }
  if (eventAdmin == 0)
  {
    BERRY_WARN << "ctkEventAdmin service not found. Unable to open context help";
  }
  else
  {
    ctkEvent ev("org/blueberry/ui/help/CONTEXTHELP_REQUESTED");
    eventAdmin->postEvent(ev);
  }
  return Object::Pointer();
}

}

