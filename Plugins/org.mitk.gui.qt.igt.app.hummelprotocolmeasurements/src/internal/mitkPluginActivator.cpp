/*=========================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

=========================================================================*/

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
