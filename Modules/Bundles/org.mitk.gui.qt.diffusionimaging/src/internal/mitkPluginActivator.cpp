#include "mitkPluginActivator.h"

#include <QtPlugin>


#include "src/internal/QmitkDiffusionImagingPublicPerspective.h"

#include "src/internal/QmitkQBallReconstructionView.h"
#include "src/internal/QmitkPreprocessingView.h"
#include "src/internal/QmitkDiffusionDicomImportView.h"
#include "src/internal/QmitkDiffusionQuantificationView.h"
#include "src/internal/QmitkTensorReconstructionView.h"
#include "src/internal/QmitkControlVisualizationPropertiesView.h"
#include "src/internal/QmitkODFDetailsView.h"
#include "src/internal/QmitkGlobalFiberTrackingView.h"
#include "src/internal/QmitkFiberBundleOperationsView.h"

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDiffusionImagingPublicPerspective, context)

  BERRY_REGISTER_EXTENSION_CLASS(QmitkQBallReconstructionView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkPreprocessingView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDiffusionDicomImport, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDiffusionQuantificationView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkTensorReconstructionView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkControlVisualizationPropertiesView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkODFDetailsView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkGlobalFiberTrackingView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkFiberBundleOperationsView, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_diffusionimaging, mitk::PluginActivator)
