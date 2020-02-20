/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkBoundingShapeObjectFactory.h>

#include "mitkPluginActivator.h"
#include "QmitkImageCropperView.h"

void mitk::mitkPluginActivator::start(ctkPluginContext* context)
{
  RegisterBoundingShapeObjectFactory();
  BERRY_REGISTER_EXTENSION_CLASS(QmitkImageCropperView, context)
}

void mitk::mitkPluginActivator::stop(ctkPluginContext*)
{
}
