/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryDynamicHelpHandler.h"

#include "berryWorkbenchPlugin.h"

#include <service/event/ctkEventAdmin.h>
#include <ctkPluginException.h>

namespace berry {

Object::Pointer DynamicHelpHandler::Execute(const SmartPointer<const ExecutionEvent>& /*event*/)
{
  ctkPluginContext* context = WorkbenchPlugin::GetDefault()->GetPluginContext();
  if (context == nullptr)
  {
    BERRY_WARN << "Plugin context not set, unable to open context help";
    return Object::Pointer();
  }

  // Check if the org.blueberry.ui.qt.help plug-in is installed and started
  QList<QSharedPointer<ctkPlugin> > plugins = context->getPlugins();
  foreach(QSharedPointer<ctkPlugin> p, plugins)
  {
    if (p->getSymbolicName() == "org.blueberry.ui.qt.help")
    {
      if (p->getState() != ctkPlugin::ACTIVE)
      {
        // try to activate the plug-in explicitly
        try
        {
          p->start(ctkPlugin::START_TRANSIENT);
        }
        catch (const ctkPluginException& pe)
        {
          BERRY_ERROR << "Activating org.blueberry.ui.qt.help failed: " << pe.what();
          return Object::Pointer();
        }
      }
    }
  }

  ctkServiceReference eventAdminRef = context->getServiceReference<ctkEventAdmin>();
  ctkEventAdmin* eventAdmin = nullptr;
  if (eventAdminRef)
  {
    eventAdmin = context->getService<ctkEventAdmin>(eventAdminRef);
  }
  if (eventAdmin == nullptr)
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

