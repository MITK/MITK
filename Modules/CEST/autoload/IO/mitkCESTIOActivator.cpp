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

#include "mitkCESTIOActivator.h"

#include "mitkCESTDICOMReaderService.h"

#include <usModuleContext.h>

#include "mitkCESTIOMimeTypes.h"

namespace mitk
{
  void CESTIOActivator::Load(us::ModuleContext *context)
  {
    m_CESTDICOMReader.reset(new CESTDICOMReaderService());

    us::ServiceProperties props;
    props[us::ServiceConstants::SERVICE_RANKING()] = 10;

    m_MimeTypes = mitk::MitkCESTIOMimeTypes::Get();
    for (std::vector<mitk::CustomMimeType *>::const_iterator mimeTypeIter = m_MimeTypes.begin(),
                                                             iterEnd = m_MimeTypes.end();
         mimeTypeIter != iterEnd;
         ++mimeTypeIter)
    {
      context->RegisterService(*mimeTypeIter, props);
    }
  }

  void CESTIOActivator::Unload(us::ModuleContext *) {}
}

US_EXPORT_MODULE_ACTIVATOR(mitk::CESTIOActivator)
