/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSceneFileReader.h"
#include "mitkSceneJsonFileReader.h"

#include <usModuleActivator.h>
#include <usModuleContext.h>

#include <memory>

namespace mitk
{
  /**
   * \brief Registers scene-file readers contributed by SceneSerialization.
   *
   * Makes `.mitk` (ZIP archive) and `.mitkscene.json` (standalone JSON)
   * scene files loadable through the regular microservice file-reader
   * registry (i.e. via IOUtil::Load, QmitkIOUtil::Load, and the File >
   * Open action). Both readers ultimately delegate to the in-module scene
   * loaders (SceneIO resp. SceneJsonReader).
   */
  class SceneSerializationIOModuleActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext *) override
    {
      m_SceneReader.reset(new SceneFileReader());
      m_SceneJsonReader.reset(new SceneJsonFileReader());
    }

    void Unload(us::ModuleContext *) override
    {
      m_SceneJsonReader.reset();
      m_SceneReader.reset();
    }

  private:
    std::unique_ptr<SceneFileReader> m_SceneReader;
    std::unique_ptr<SceneJsonFileReader> m_SceneJsonReader;
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::SceneSerializationIOModuleActivator)
