/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkPluginActivator.h"

#include "QmitkSegmentationView.h"
#include "QmitkSegmentationPreferencePage.h"
#include "QmitkSegmentationUtilitiesView.h"
#include "QmitkSegmentationTaskListView.h"

#include "QmitkAutocropAction.h"
#include "QmitkAutocropLabelSetImageAction.h"
#include "QmitkCreatePolygonModelAction.h"
#include "QmitkLoadMultiLabelPresetAction.h"
#include "QmitkSaveMultiLabelPresetAction.h"
#include "QmitkConvertSurfaceToLabelAction.h"
#include "QmitkConvertMaskToLabelAction.h"
#include "QmitkConvertToMultiLabelSegmentationAction.h"
#include "QmitkCreateMultiLabelSegmentationAction.h"

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

using namespace mitk;

ctkPluginContext* PluginActivator::m_context = nullptr;
PluginActivator* PluginActivator::m_Instance = nullptr;

PluginActivator::PluginActivator()
{
  m_Instance = this;
}

PluginActivator::~PluginActivator()
{
  m_Instance = nullptr;
}

void PluginActivator::start(ctkPluginContext *context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSegmentationView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSegmentationPreferencePage, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSegmentationUtilitiesView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSegmentationTaskListView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkAutocropAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkAutocropLabelSetImageAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkCreatePolygonModelAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkLoadMultiLabelPresetAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSaveMultiLabelPresetAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkConvertSurfaceToLabelAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkConvertMaskToLabelAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkConvertToMultiLabelSegmentationAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkCreateMultiLabelSegmentationAction, context)

  this->m_context = context;
}

void PluginActivator::stop(ctkPluginContext *)
{
  this->m_context = nullptr;
}

PluginActivator* PluginActivator::getDefault()
{
  return m_Instance;
}

ctkPluginContext*PluginActivator::getContext()
{
  return m_context;
}
