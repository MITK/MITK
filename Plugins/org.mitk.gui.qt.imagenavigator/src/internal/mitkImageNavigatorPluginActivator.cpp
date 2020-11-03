/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkImageNavigatorPluginActivator.h"
#include "QmitkImageNavigatorView.h"

namespace mitk {

  void ImageNavigatorPluginActivator::start(ctkPluginContext* context)
  {
    BERRY_REGISTER_EXTENSION_CLASS(QmitkImageNavigatorView, context)
  }

  void ImageNavigatorPluginActivator::stop(ctkPluginContext* context)
  {
    Q_UNUSED(context)
  }

}
