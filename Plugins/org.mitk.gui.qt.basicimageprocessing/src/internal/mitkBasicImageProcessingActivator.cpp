/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBasicImageProcessingActivator.h"
#include "QmitkBasicImageProcessingView.h"

namespace mitk {

void BasicImageProcessingActivator::start(ctkPluginContext* context)
{
   BERRY_REGISTER_EXTENSION_CLASS(QmitkBasicImageProcessing, context)
}

void BasicImageProcessingActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}
