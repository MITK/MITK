/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourModelReader.h"
#include "mitkContourModelSetReader.h"
#include "mitkContourModelSetWriter.h"
#include "mitkContourModelWriter.h"

#include <mitkBaseRenderer.h>
#include <mitkContourModel.h>
#include <mitkContourModelMapper2D.h>
#include <mitkContourModelMapper3D.h>
#include <mitkContourModelSet.h>
#include <mitkContourModelSetMapper2D.h>
#include <mitkContourModelSetMapper3D.h>
#include <mitkMapperProviderBase.h>

#include <usModuleActivator.h>
#include <usModuleContext.h>

#include <memory>
#include <vector>

namespace mitk
{
  /*
   * This is the module activator for the "ContourModel" module.
   */
  class ContourModelActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext * /*context*/) override
    {
      m_ContourModelReader = new ContourModelReader();
      m_ContourModelSetReader = new ContourModelSetReader();
      m_ContourModelWriter = new ContourModelWriter();
      m_ContourModelSetWriter = new ContourModelSetWriter();

      m_MapperProviders.push_back(
        std::make_unique<MapperProviderBase<ContourModelMapper2D, ContourModel>>(BaseRenderer::Standard2D));
      m_MapperProviders.push_back(
        std::make_unique<MapperProviderBase<ContourModelMapper3D, ContourModel>>(BaseRenderer::Standard3D));
      m_MapperProviders.push_back(
        std::make_unique<MapperProviderBase<ContourModelSetMapper2D, ContourModelSet>>(BaseRenderer::Standard2D));
      m_MapperProviders.push_back(
        std::make_unique<MapperProviderBase<ContourModelSetMapper3D, ContourModelSet>>(BaseRenderer::Standard3D));
    }

    void Unload(us::ModuleContext *) override
    {
      m_MapperProviders.clear();

      delete m_ContourModelReader;
      delete m_ContourModelSetReader;
      delete m_ContourModelWriter;
      delete m_ContourModelSetWriter;
    }

  private:
    mitk::ContourModelReader *m_ContourModelReader;
    mitk::ContourModelSetReader *m_ContourModelSetReader;
    mitk::ContourModelWriter *m_ContourModelWriter;
    mitk::ContourModelSetWriter *m_ContourModelSetWriter;

    std::vector<std::unique_ptr<IMapperProvider>> m_MapperProviders;
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::ContourModelActivator)
