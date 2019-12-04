/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <usModuleActivator.h>
#include <usModuleContext.h>

#include <mitkExampleDataStructureReaderService.h>

#include <mitkExampleDataStructureWriterService.h>

#include "mitkExampleIOMimeTypes.h"

namespace mitk
{
  /**
  \brief Registers services for example module.
  */
  class NewModuleIOActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext *context) override
    {
      // We can register our read/write services with a custom service ranking
      // services with a higher ranking are prioritized, default us 0
      us::ServiceProperties props;
      props[us::ServiceConstants::SERVICE_RANKING()] = 10;

      m_MimeTypes = mitk::ExampleIOMimeTypes::Get();
      for (std::vector<mitk::CustomMimeType *>::const_iterator mimeTypeIter = m_MimeTypes.begin(),
                                                               iterEnd = m_MimeTypes.end();
           mimeTypeIter != iterEnd;
           ++mimeTypeIter)
      {
        context->RegisterService(*mimeTypeIter, props);
      }

      m_ExampleDataStructureReaderService = new ExampleDataStructureReaderService();

      m_ExampleDataStructureWriterService = new ExampleDataStructureWriterService();
    }

    void Unload(us::ModuleContext *) override
    {
      for (unsigned int loop(0); loop < m_MimeTypes.size(); ++loop)
      {
        delete m_MimeTypes.at(loop);
      }

      delete m_ExampleDataStructureReaderService;

      delete m_ExampleDataStructureWriterService;
    }

  private:
    ExampleDataStructureReaderService *m_ExampleDataStructureReaderService;

    ExampleDataStructureWriterService *m_ExampleDataStructureWriterService;

    std::vector<mitk::CustomMimeType *> m_MimeTypes;
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::NewModuleIOActivator)
