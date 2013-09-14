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
#include "src/internal/QmitkFiberProcessingView.h"
//#include "src/internal/QmitkFiberBundleDeveloperView.h"
#include "src/internal/QmitkPartialVolumeAnalysisView.h"
#include "src/internal/QmitkIVIMView.h"
#include "src/internal/QmitkTractbasedSpatialStatisticsView.h"
#include "src/internal/QmitkTbssSkeletonizationView.h"
#include "src/internal/QmitkStreamlineTrackingView.h"
#include "src/internal/Connectomics/QmitkConnectomicsDataView.h"
#include "src/internal/Connectomics/QmitkConnectomicsNetworkOperationsView.h"
#include "src/internal/Connectomics/QmitkConnectomicsStatisticsView.h"
#include "src/internal/QmitkOdfMaximaExtractionView.h"
#include "src/internal/QmitkFiberfoxView.h"
#include "src/internal/QmitkFiberExtractionView.h"
#include "src/internal/QmitkFieldmapGeneratorView.h"
#include "src/internal/QmitkDiffusionRegistrationView.h"

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
  BERRY_REGISTER_EXTENSION_CLASS(QmitkFiberProcessingView, context)
//  BERRY_REGISTER_EXTENSION_CLASS(QmitkFiberBundleDeveloperView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkPartialVolumeAnalysisView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkIVIMView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkTractbasedSpatialStatisticsView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkTbssSkeletonizationView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkConnectomicsDataView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkConnectomicsNetworkOperationsView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkConnectomicsStatisticsView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkStreamlineTrackingView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkOdfMaximaExtractionView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkFiberfoxView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkFiberExtractionView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkFieldmapGeneratorView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDiffusionRegistrationView, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_diffusionimaging, mitk::PluginActivator)
