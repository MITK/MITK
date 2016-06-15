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

#include <mitkDiffusionImageNrrdReaderService.h>
#include <mitkDiffusionImageNiftiReaderService.h>
#include <mitkNrrdTensorImageReader.h>
#include <mitkNrrdQBallImageReader.h>

#include <mitkDiffusionImageNrrdWriterService.h>
#include <mitkDiffusionImageNiftiWriterService.h>
#include <mitkNrrdTensorImageWriter.h>
#include <mitkNrrdQBallImageWriter.h>

#include <mitkDiffusionPropertyHelper.h>

#include <mitkCoreServices.h>
#include <mitkIPropertyDescriptions.h>
#include <mitkIPropertyPersistence.h>

#include "mitkDiffusionCoreIOMimeTypes.h"

namespace mitk
{
  /**
  \brief Registers services for segmentation module.
  */
  class DiffusionCoreIOActivator : public us::ModuleActivator
  {
  public:

    void Load(us::ModuleContext* context) override
    {
      us::ServiceProperties props;
      props[ us::ServiceConstants::SERVICE_RANKING() ] = 10;

      m_MimeTypes = mitk::DiffusionCoreIOMimeTypes::Get();
      for (std::vector<mitk::CustomMimeType*>::const_iterator mimeTypeIter = m_MimeTypes.begin(),
        iterEnd = m_MimeTypes.end(); mimeTypeIter != iterEnd; ++mimeTypeIter)
      {
        context->RegisterService(*mimeTypeIter, props);
      }

      m_DiffusionImageNrrdReaderService = new DiffusionImageNrrdReaderService();
      m_DiffusionImageNiftiReaderService = new DiffusionImageNiftiReaderService();
      m_NrrdTensorImageReader = new NrrdTensorImageReader();
      m_NrrdQBallImageReader = new NrrdQBallImageReader();

      m_DiffusionImageNrrdWriterService = new DiffusionImageNrrdWriterService();
      m_DiffusionImageNiftiWriterService = new DiffusionImageNiftiWriterService();
      m_NrrdTensorImageWriter = new NrrdTensorImageWriter();
      m_NrrdQBallImageWriter = new NrrdQBallImageWriter();

      //register relevant properties
      //non-persistent properties
      mitk::CoreServices::GetPropertyDescriptions()->AddDescription(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME, "This map stores which b values belong to which gradients.");
      mitk::CoreServices::GetPropertyDescriptions()->AddDescription(mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME, "The original gradients used during acquisition. This property may be empty.");
      //persistent properties
      mitk::CoreServices::GetPropertyDescriptions()->AddDescription(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME, "The reference b value the gradients are normalized to.");
      mitk::CoreServices::GetPropertyDescriptions()->AddDescription(mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME, "The measurment frame used during acquisition.");
      mitk::CoreServices::GetPropertyDescriptions()->AddDescription(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME, "The gradients used during acquisition.");
      mitk::CoreServices::GetPropertyDescriptions()->AddDescription(mitk::DiffusionPropertyHelper::MODALITY, "Defines the modality used for acquisition. DWMRI signifies diffusion weighted images.");

      mitk::CoreServices::GetPropertyPersistence()->AddInfo(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME, mitk::PropertyPersistenceInfo::New("DWMRI_b-value"));
      mitk::CoreServices::GetPropertyPersistence()->AddInfo(mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME, mitk::PropertyPersistenceInfo::New("measurement frame"));
      mitk::CoreServices::GetPropertyPersistence()->AddInfo(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME, mitk::PropertyPersistenceInfo::New("DWMRI_gradient"));
      mitk::CoreServices::GetPropertyPersistence()->AddInfo(mitk::DiffusionPropertyHelper::MODALITY, mitk::PropertyPersistenceInfo::New("modality"));
    }

    void Unload(us::ModuleContext*) override
    {
      for (unsigned int loop(0); loop < m_MimeTypes.size(); ++loop)
      {
        delete m_MimeTypes.at(loop);
      }

      delete m_DiffusionImageNrrdReaderService;
      delete m_DiffusionImageNiftiReaderService;
      delete m_NrrdTensorImageReader;
      delete m_NrrdQBallImageReader;

      delete m_DiffusionImageNrrdWriterService;
      delete m_DiffusionImageNiftiWriterService;
      delete m_NrrdTensorImageWriter;
      delete m_NrrdQBallImageWriter;
    }

  private:

    DiffusionImageNrrdReaderService * m_DiffusionImageNrrdReaderService;
    DiffusionImageNiftiReaderService * m_DiffusionImageNiftiReaderService;
    NrrdTensorImageReader * m_NrrdTensorImageReader;
    NrrdQBallImageReader * m_NrrdQBallImageReader;

    DiffusionImageNrrdWriterService * m_DiffusionImageNrrdWriterService;
    DiffusionImageNiftiWriterService * m_DiffusionImageNiftiWriterService;
    NrrdTensorImageWriter * m_NrrdTensorImageWriter;
    NrrdQBallImageWriter * m_NrrdQBallImageWriter;

    std::vector<mitk::CustomMimeType*> m_MimeTypes;

  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::DiffusionCoreIOActivator)
