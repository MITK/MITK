/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPluginActivator.h"
#include "QmitkSemanticRelationsStatisticsView.h"
#include "QmitkSemanticRelationsView.h"

#include <mitkPersistenceService.h>

namespace mitk
{
  void SemanticRelationsActivator::start(ctkPluginContext *context)
  {
    mitk::PersistenceService::LoadModule();

    BERRY_REGISTER_EXTENSION_CLASS(QmitkSemanticRelationsView, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkSemanticRelationsStatisticsView, context)
  }

  void SemanticRelationsActivator::stop(ctkPluginContext *context) { Q_UNUSED(context) }
}
