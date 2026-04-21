/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSceneJsonMimeType.h"

#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usServiceProperties.h>

namespace mitk
{
  /**
   * \brief Registers MIME types contributed by SceneSerialization.
   *
   * Intentionally does not register any AbstractFileReader / Writer: scene
   * loading is dispatched through SceneIO, not the microservice-based file
   * reader registry (see AbstractSceneReader).
   */
  class SceneSerializationIOModuleActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext *context) override
    {
      us::ServiceProperties props;
      props[us::ServiceConstants::SERVICE_RANKING()] = 10;

      auto *mimeType = new SceneJsonMimeType();
      context->RegisterService(static_cast<CustomMimeType *>(mimeType), props);
    }

    void Unload(us::ModuleContext *) override
    {
    }
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::SceneSerializationIOModuleActivator)
