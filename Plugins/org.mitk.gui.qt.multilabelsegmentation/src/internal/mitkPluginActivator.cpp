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
#include "QmitkConvertSurfaceToLabelAction.h"
#include "QmitkConvertMaskToLabelAction.h"
#include "QmitkConvertToMultiLabelSegmentationAction.h"
#include "QmitkCreateMultiLabelSegmentationAction.h"
#include "QmitkMultiLabelSegmentationPreferencePage.h"
#include "SegmentationUtilities/QmitkMultiLabelSegmentationUtilitiesView.h"

#include <usModuleInitialization.h>

ctkPluginContext* mitk::PluginActivator::m_Context = NULL;

void mitk::PluginActivator::start(ctkPluginContext *context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkMultiLabelSegmentationView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkThresholdAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkCreatePolygonModelAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkAutocropAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkConvertSurfaceToLabelAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkConvertMaskToLabelAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkConvertToMultiLabelSegmentationAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkCreateMultiLabelSegmentationAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkMultiLabelSegmentationPreferencePage, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkMultiLabelSegmentationUtilitiesView, context)

  m_Context = context;
}

void mitk::PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
  m_Context = NULL;
}

ctkPluginContext* mitk::PluginActivator::getContext()
{
  return m_Context;
}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_multilabelsegmentation, mitk::PluginActivator)

US_INITIALIZE_MODULE("MultiLabelSegmentation", "org_mitk_gui_qt_multilabelsegmentation")
