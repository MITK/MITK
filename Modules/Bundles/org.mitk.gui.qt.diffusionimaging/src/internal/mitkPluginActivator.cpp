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
#include "src/internal/QmitkGibbsTrackingView.h"
#include "src/internal/QmitkStochasticFiberTrackingView.h"
#include "src/internal/QmitkFiberBundleOperationsView.h"
#include "src/internal/QmitkFiberBundleDeveloperView.h"
#include "src/internal/QmitkPartialVolumeAnalysisView.h"
#include "src/internal/QmitkIVIMView.h"
#include "src/internal/QmitkScreenshotMaker.h"

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
  BERRY_REGISTER_EXTENSION_CLASS(QmitkGibbsTrackingView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkStochasticFiberTrackingView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkFiberBundleOperationsView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkFiberBundleDeveloperView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkPartialVolumeAnalysisView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkIVIMView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkScreenshotMaker, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_diffusionimaging, mitk::PluginActivator)
