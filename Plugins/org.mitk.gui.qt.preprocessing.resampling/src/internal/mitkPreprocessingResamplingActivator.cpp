/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPreprocessingResamplingActivator.h"
#include "QmitkPreprocessingResamplingView.h"

namespace mitk {

void PreprocessingResamplingActivator::start(ctkPluginContext* context)
{
   BERRY_REGISTER_EXTENSION_CLASS(QmitkPreprocessingResampling, context)
}

void PreprocessingResamplingActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}
