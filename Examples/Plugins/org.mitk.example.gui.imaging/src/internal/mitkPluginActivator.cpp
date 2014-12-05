/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#include "mitkPluginActivator.h"

#include <QtPlugin>


#include "src/internal/colourimageprocessing/QmitkColourImageProcessingView.h"
#include "src/internal/isosurface/QmitkIsoSurface.h"
#include "src/internal/simpleexample/QmitkSimpleExampleView.h"
#include "src/internal/simplemeasurement/QmitkSimpleMeasurement.h"
#include "src/internal/viewinitialization/QmitkViewInitializationView.h"
#include "src/internal/volumetry/QmitkVolumetryView.h"
#include "src/internal/surfaceutilities/QmitkSurfaceUtilities.h"

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{

  BERRY_REGISTER_EXTENSION_CLASS(QmitkColourImageProcessingView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkIsoSurface, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSimpleExampleView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSimpleMeasurement, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkViewInitializationView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkVolumetryView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSurfaceUtilities, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_gui_qt_examples, mitk::PluginActivator)
#endif
