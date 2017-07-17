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
#include <usModuleActivator.h>
#include <usModuleContext.h>

#include <mitkRTDoseReaderService.h>
#include <mitkRTPlanReaderService.h>
#include <mitkRTStructureSetReaderService.h>

#include <mitkCoreServices.h>
#include <mitkIPropertyDescriptions.h>
#include <mitkIPropertyPersistence.h>

#include "mitkDicomRTIOMimeTypes.h"

namespace mitk
{
  /**
  \brief Registers services for segmentation module.
  */
  class DicomRTIOActivator : public us::ModuleActivator
  {
  public:

    void Load(us::ModuleContext* context) override
    {
      us::ServiceProperties props;
      props[ us::ServiceConstants::SERVICE_RANKING() ] = 100;

      m_MimeTypes = mitk::DicomRTIOMimeTypes::Get();
      for (std::vector<mitk::CustomMimeType*>::const_iterator mimeTypeIter = m_MimeTypes.begin(),
        iterEnd = m_MimeTypes.end(); mimeTypeIter != iterEnd; ++mimeTypeIter)
      {
        context->RegisterService(*mimeTypeIter, props);
      }

      m_RTDoseReader = new RTDoseReaderService();
      m_RTPlanReader = new RTPlanReaderService();
      m_RTStructureSetReader = new RTStructureSetReaderService();
    }

    void Unload(us::ModuleContext*) override
    {
      for (auto& aMimeType : m_MimeTypes)
      {
        delete aMimeType;
      }

      delete m_RTDoseReader;
      delete m_RTPlanReader;
      delete m_RTStructureSetReader;
    }

  private:

    RTDoseReaderService * m_RTDoseReader;
    RTPlanReaderService * m_RTPlanReader;
    RTStructureSetReaderService * m_RTStructureSetReader;

    std::vector<mitk::CustomMimeType*> m_MimeTypes;

  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::DicomRTIOActivator)
