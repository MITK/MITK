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

#include "src/internal/Perspectives/QmitkFiberProcessingPerspective.h"

#include "src/internal/QmitkFiberProcessingView.h"
#include "src/internal/QmitkFiberQuantificationView.h"
#include "src/internal/QmitkFiberClusteringView.h"
#include "src/internal/QmitkFiberFitView.h"
#include "src/internal/QmitkTractometryView.h"


ctkPluginContext* mitk::PluginActivator::m_Context = nullptr;

ctkPluginContext* mitk::PluginActivator::GetContext()
{
  return m_Context;
}

void mitk::PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkFiberProcessingPerspective, context)

  BERRY_REGISTER_EXTENSION_CLASS(QmitkFiberQuantificationView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkFiberProcessingView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkFiberClusteringView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkFiberFitView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkTractometryView, context)
  m_Context = context;
}

void mitk::PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
  m_Context = nullptr;
}
