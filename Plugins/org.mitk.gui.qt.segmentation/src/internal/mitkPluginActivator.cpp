/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkPluginActivator.h"

#include "QmitkSegmentationView.h"
#include "QmitkThresholdAction.h"
#include "QmitkCreatePolygonModelAction.h"
#include "QmitkAutocropAction.h"
#include "QmitkAutocropLabelSetImageAction.h"
#include "QmitkSegmentationPreferencePage.h"
#include "SegmentationUtilities/QmitkSegmentationUtilitiesView.h"

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
  BERRY_REGISTER_EXTENSION_CLASS(QmitkThresholdAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkCreatePolygonModelAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkAutocropAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkAutocropLabelSetImageAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSegmentationPreferencePage, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSegmentationUtilitiesView, context)

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
