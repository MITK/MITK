/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkBoundingShapeObjectFactory.h>

#include "org_mitk_gui_qt_imagecropper_Activator.h"
#include "QmitkImageCropper.h"

void mitk::org_mitk_gui_qt_imagecropper_Activator::start(ctkPluginContext* context)
{
  RegisterBoundingShapeObjectFactory();
  BERRY_REGISTER_EXTENSION_CLASS(QmitkImageCropper, context)
}

void mitk::org_mitk_gui_qt_imagecropper_Activator::stop(ctkPluginContext*)
{
}
