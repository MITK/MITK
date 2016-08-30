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
#include "src/internal/videoplayer/QmitkVideoPlayer.h"
#include <mitkPersistenceService.h>

namespace mitk {

  void PluginActivator::start(ctkPluginContext* context)
  {
    mitk::PersistenceService::LoadModule();

    BERRY_REGISTER_EXTENSION_CLASS(QmitkVideoPlayer, context)
  }

  void PluginActivator::stop(ctkPluginContext* context)
  {
    Q_UNUSED(context)
  }

}
