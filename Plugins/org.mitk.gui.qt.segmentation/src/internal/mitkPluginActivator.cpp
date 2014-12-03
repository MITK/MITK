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
#include "QmitkThresholdAction.h"
#include "QmitkCreatePolygonModelAction.h"
#include "QmitkAutocropAction.h"
#include "QmitkSegmentationPreferencePage.h"
#include "QmitkDeformableClippingPlaneView.h"
#include "SegmentationUtilities/QmitkSegmentationUtilitiesView.h"

using namespace mitk;

ctkPluginContext* PluginActivator::m_context = NULL;

void PluginActivator::start(ctkPluginContext *context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSegmentationView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkThresholdAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkCreatePolygonModelAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkAutocropAction, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSegmentationPreferencePage, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDeformableClippingPlaneView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSegmentationUtilitiesView, context)

  this->m_context = context;
}

void PluginActivator::stop(ctkPluginContext *)
{
  this->m_context = NULL;
}

ctkPluginContext*PluginActivator::getContext()
{
  return m_context;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_gui_qt_segmentation, mitk::PluginActivator)
#endif
