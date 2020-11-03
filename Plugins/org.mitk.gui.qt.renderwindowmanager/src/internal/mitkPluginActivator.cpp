/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPluginActivator.h"
#include "QmitkRenderWindowManagerView.h"

namespace mitk
{
  void RenderWindowManagerActivator::start(ctkPluginContext *context)
  {
    BERRY_REGISTER_EXTENSION_CLASS(QmitkRenderWindowManagerView, context)
  }

  void RenderWindowManagerActivator::stop(ctkPluginContext *context) { Q_UNUSED(context) }
}
