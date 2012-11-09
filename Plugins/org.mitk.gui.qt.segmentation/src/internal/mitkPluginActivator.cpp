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

#include "QmitkSegmentationView.h"
#include "QmitkBooleanOperationsView.h"
#include "QmitkThresholdAction.h"
#include "QmitkOtsuAction.h"
#include "QmitkCreatePolygonModelAction.h"
#include "QmitkStatisticsAction.h"
#include "QmitkAutocropAction.h"
#include "QmitkSegmentationPreferencePage.h"
#include "QmitkDeformableClippingPlaneView.h"
#include "src/internal/regiongrowing/QmitkRegionGrowingView.h"

using namespace mitk;

void PluginActivator::start(ctkPluginContext *context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSegmentationView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkBooleanOperationsView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkThresholdAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkOtsuAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkCreatePolygonModelAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkStatisticsAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkAutocropAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSegmentationPreferencePage, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDeformableClippingPlaneView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkRegionGrowingView, context);
}

void PluginActivator::stop(ctkPluginContext *)
{
}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_segmentation, mitk::PluginActivator)
