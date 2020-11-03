/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPluginActivator.h"
#include "QmitkIGTTrackingSemiAutomaticMeasurementView.h"
#include "QmitkIGTTrackingDataEvaluationView.h"

void mitk::PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkIGTTrackingSemiAutomaticMeasurementView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkIGTTrackingDataEvaluationView, context)
}

void mitk::PluginActivator::stop(ctkPluginContext*)
{
}
