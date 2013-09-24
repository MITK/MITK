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

#include "QmitkMultiLabelSegmentationView.h"
#include "QmitkThresholdAction.h"
#include "QmitkCreatePolygonModelAction.h"
#include "QmitkAutocropAction.h"
#include "QmitkConvertToMultiLabelSegmentationAction.h"
#include "QmitkCreateMultiLabelSegmentationAction.h"
#include "QmitkMultiLabelSegmentationPreferencePage.h"
#include "SegmentationUtilities/QmitkMultiLabelSegmentationUtilitiesView.h"

using namespace mitk;

void PluginActivator::start(ctkPluginContext *context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkMultiLabelSegmentationView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkThresholdAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkCreatePolygonModelAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkAutocropAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkConvertToMultiLabelSegmentationAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkCreateMultiLabelSegmentationAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkMultiLabelSegmentationPreferencePage, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkMultiLabelSegmentationUtilitiesView, context)
}

void PluginActivator::stop(ctkPluginContext *)
{
}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_multilabelsegmentation, mitk::PluginActivator)
