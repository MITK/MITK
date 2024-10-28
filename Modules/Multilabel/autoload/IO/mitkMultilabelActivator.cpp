/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModuleActivator.h>
#include <usModuleContext.h>

#include "mitkLegacyLabelSetImageIO.h"
#include "mitkMultiLabelSegmentationIO.h"
#include "mitkMultilabelIOMimeTypes.h"

#include <memory>
#include <vector>

namespace mitk
{
  /**
  \brief Registers services for multilabel module.
  */
  class MultilabelIOModuleActivator : public us::ModuleActivator
  {
    std::vector<std::unique_ptr<AbstractFileIO>> m_FileIOs;
    std::unique_ptr<IFileReader> m_LegacyLabelSetImageIOReader;

  public:
    void Load(us::ModuleContext *context) override
    {
      auto mimeTypes = MitkMultilabelIOMimeTypes::Get();

      us::ServiceProperties props;
      props[us::ServiceConstants::SERVICE_RANKING()] = 10;

      for (const auto &mimeType : mimeTypes)
        context->RegisterService(mimeType, props);

      m_LegacyLabelSetImageIOReader = std::make_unique<LegacyLabelSetImageIO>();

      m_FileIOs.push_back(std::make_unique<MultiLabelSegmentationIO>());
    }

    void Unload(us::ModuleContext *) override
    {
    }
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::MultilabelIOModuleActivator)
