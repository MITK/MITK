/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkPluginActivator.h"

#include "colourimageprocessing/QmitkColourImageProcessingView.h"
#include "isosurface/QmitkIsoSurface.h"
#include "simpleexample/QmitkSimpleExampleView.h"
#include "simplemeasurement/QmitkSimpleMeasurement.h"
#include "surfaceutilities/QmitkSurfaceUtilities.h"
#include "viewinitialization/QmitkViewInitializationView.h"
#include "volumetry/QmitkVolumetryView.h"

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

namespace mitk
{
  void PluginActivator::start(ctkPluginContext *context)
  {
    BERRY_REGISTER_EXTENSION_CLASS(QmitkColourImageProcessingView, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkIsoSurface, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkSimpleExampleView, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkSimpleMeasurement, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkViewInitializationView, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkVolumetryView, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkSurfaceUtilities, context)
  }

  void PluginActivator::stop(ctkPluginContext *context) { Q_UNUSED(context) }
}
