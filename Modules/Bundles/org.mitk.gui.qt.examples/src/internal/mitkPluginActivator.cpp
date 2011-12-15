#include "mitkPluginActivator.h"

#include <QtPlugin>


#include "src/internal/colourimageprocessing/QmitkColourImageProcessingView.h"
#include "src/internal/isosurface/QmitkIsoSurface.h"
#include "src/internal/regiongrowing/QmitkRegionGrowingView.h"
#include "src/internal/simpleexample/QmitkSimpleExampleView.h"
#include "src/internal/simplemeasurement/QmitkSimpleMeasurement.h"
#include "src/internal/viewinitialization/QmitkViewInitializationView.h"
#include "src/internal/volumetry/QmitkVolumetryView.h"

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{

  BERRY_REGISTER_EXTENSION_CLASS(QmitkColourImageProcessingView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkIsoSurface, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkRegionGrowingView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSimpleExampleView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSimpleMeasurement, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkViewInitializationView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkVolumetryView, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_examples, mitk::PluginActivator)
