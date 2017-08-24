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

#include "src/internal/QmitkConnectomicsDataView.h"
#include "src/internal/QmitkConnectomicsNetworkOperationsView.h"
#include "src/internal/QmitkConnectomicsStatisticsView.h"
#include "src/internal/QmitkRandomParcellationView.h"
#include "src/internal/Perspectives/QmitkConnectomicsPerspective.h"

ctkPluginContext* mitk::PluginActivator::m_Context = nullptr;

ctkPluginContext* mitk::PluginActivator::GetContext()
{
  return m_Context;
}

void mitk::PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkConnectomicsPerspective, context)
  
  BERRY_REGISTER_EXTENSION_CLASS(QmitkConnectomicsDataView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkConnectomicsNetworkOperationsView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkConnectomicsStatisticsView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkRandomParcellationView, context)
  m_Context = context;
}

void mitk::PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
  m_Context = nullptr;
}
