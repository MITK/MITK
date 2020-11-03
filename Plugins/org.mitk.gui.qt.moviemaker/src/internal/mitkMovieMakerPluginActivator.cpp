/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMovieMakerPluginActivator.h"
#include "QmitkMovieMakerView.h"
#include "QmitkScreenshotMaker.h"

namespace mitk {

  void MovieMakerPluginActivator::start(ctkPluginContext* context)
  {
    BERRY_REGISTER_EXTENSION_CLASS(QmitkMovieMakerView, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkScreenshotMaker, context)
  }

  void MovieMakerPluginActivator::stop(ctkPluginContext* context)
  {
    Q_UNUSED(context)
  }

}
