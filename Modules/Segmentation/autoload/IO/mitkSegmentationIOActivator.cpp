/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModuleActivator.h>
#include <usModuleContext.h>

#include "mitkSegmentationIOMimeTypes.h"
#include "mitkSegmentationTaskListIO.h"

#include <memory>
#include <vector>

namespace mitk
{
  class SegmentationIOModuleActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext* context) override
    {
      auto mimeTypes = MitkSegmentationIOMimeTypes::Get();

      us::ServiceProperties props;
      props[us::ServiceConstants::SERVICE_RANKING()] = 10;

      for (const auto& mimeType : mimeTypes)
        context->RegisterService(mimeType, props);

      m_FileIOs.push_back(std::make_unique<SegmentationTaskListIO>());
    }

    void Unload(us::ModuleContext*) override
    {
    }

  private:
    std::vector<std::unique_ptr<AbstractFileIO>> m_FileIOs;
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::SegmentationIOModuleActivator)
