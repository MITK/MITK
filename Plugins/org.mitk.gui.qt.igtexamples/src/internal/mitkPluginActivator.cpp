/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkPluginActivator.h"
#include "QmitkIGTTutorialView.h"
#include "QmitkIGTTrackingLabView.h"
#include "OpenIGTLinkExample.h"
#include "OpenIGTLinkProviderExample.h"
#include "OpenIGTLinkPlugin.h"

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkIGTTutorialView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkIGTTrackingLabView, context)
  BERRY_REGISTER_EXTENSION_CLASS(OpenIGTLinkExample, context)
  BERRY_REGISTER_EXTENSION_CLASS(OpenIGTLinkProviderExample, context)
  BERRY_REGISTER_EXTENSION_CLASS(OpenIGTLinkPlugin, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}
