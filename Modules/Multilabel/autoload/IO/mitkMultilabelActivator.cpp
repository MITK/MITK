/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>

#include "mitkLegacyLabelSetImageIO.h"
#include "mitkMultiLabelSegmentationIO.h"
#include "mitkMultilabelIOMimeTypes.h"
#include "mitkSegmentationTaskListIO.h"

namespace mitk
{
  /**
  \brief Registers services for multilabel module.
  */
  class MultilabelIOModuleActivator : public us::ModuleActivator
  {
    std::vector<AbstractFileIO *> m_FileIOs;
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

      m_FileIOs.push_back(new MultiLabelSegmentationIO());
      m_FileIOs.push_back(new SegmentationTaskListIO);
    }
    void Unload(us::ModuleContext *) override
    {
      for (auto &elem : m_FileIOs)
      {
        delete elem;
      }
    }
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::MultilabelIOModuleActivator)
