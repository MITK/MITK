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
#include "QmitkSemanticRelationsView.h"
#include <mitkPersistenceService.h>

namespace mitk
{
  void SemanticRelationsActivator::start(ctkPluginContext *context)
  {
    mitk::PersistenceService::LoadModule();

    BERRY_REGISTER_EXTENSION_CLASS(QmitkSemanticRelationsView, context)
  }

  void SemanticRelationsActivator::stop(ctkPluginContext *context) { Q_UNUSED(context) }
}
