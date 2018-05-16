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
#include <mitkDiffusionImageDicomReaderService.h>
#include <mitkNrrdTensorImageReader.h>
#include <mitkNrrdOdfImageReader.h>
#include <mitkShImageReader.h>
#include <mitkPeakImageReader.h>

#include <mitkDiffusionImageNrrdWriterService.h>
#include <mitkDiffusionImageNiftiWriterService.h>
#include <mitkNrrdTensorImageWriter.h>
#include <mitkNrrdOdfImageWriter.h>
#include <mitkShImageWriter.h>

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
      m_MimeTypes = mitk::DiffusionCoreIOMimeTypes::Get();
      for (std::vector<mitk::CustomMimeType*>::const_iterator mimeTypeIter = m_MimeTypes.begin(),
        iterEnd = m_MimeTypes.end(); mimeTypeIter != iterEnd; ++mimeTypeIter)
      {
        us::ServiceProperties props;
        mitk::CustomMimeType* mt = *mimeTypeIter;
        if (mt->GetName()!=mitk::DiffusionCoreIOMimeTypes::PEAK_MIMETYPE_NAME() && mt->GetName()!=mitk::DiffusionCoreIOMimeTypes::SH_MIMETYPE_NAME())
          props[ us::ServiceConstants::SERVICE_RANKING() ] = 10;

        context->RegisterService(*mimeTypeIter, props);
      }

      m_DiffusionImageNrrdReaderService = new DiffusionImageNrrdReaderService();
      m_DiffusionImageNiftiReaderService = new DiffusionImageNiftiReaderService( CustomMimeType( mitk::DiffusionCoreIOMimeTypes::DWI_NIFTI_MIMETYPE() ), mitk::DiffusionCoreIOMimeTypes::DWI_NIFTI_MIMETYPE_DESCRIPTION() );
      m_DiffusionImageFslNiftiReaderService = new DiffusionImageNiftiReaderService( CustomMimeType( mitk::DiffusionCoreIOMimeTypes::DWI_FSL_MIMETYPE() ), mitk::DiffusionCoreIOMimeTypes::DWI_FSL_MIMETYPE_DESCRIPTION() );
      m_DiffusionImageDicomReaderService = new DiffusionImageDicomReaderService();

      m_NrrdTensorImageReader = new NrrdTensorImageReader();
      m_NrrdOdfImageReader = new NrrdOdfImageReader();
      m_PeakImageReader = new PeakImageReader();
      m_ShImageReader = new ShImageReader();

      m_DiffusionImageNrrdWriterService = new DiffusionImageNrrdWriterService();
      m_DiffusionImageNiftiWriterService = new DiffusionImageNiftiWriterService();
      m_NrrdTensorImageWriter = new NrrdTensorImageWriter();
      m_NrrdOdfImageWriter = new NrrdOdfImageWriter();
      m_ShImageWriter = new ShImageWriter();

      //register relevant properties
      //non-persistent properties
      mitk::CoreServices::GetPropertyDescriptions()->AddDescription(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME, "This map stores which b values belong to which gradients.");
      mitk::CoreServices::GetPropertyDescriptions()->AddDescription(mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME, "The original gradients used during acquisition. This property may be empty.");
      //persistent properties
      mitk::CoreServices::GetPropertyDescriptions()->AddDescription(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME, "The reference b value the gradients are normalized to.");
      mitk::CoreServices::GetPropertyDescriptions()->AddDescription(mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME, "The measurment frame used during acquisition.");
      mitk::CoreServices::GetPropertyDescriptions()->AddDescription(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME, "The gradients after applying measurement frame and image matrix.");
      mitk::CoreServices::GetPropertyDescriptions()->AddDescription(mitk::DiffusionPropertyHelper::MODALITY, "Defines the modality used for acquisition. DWMRI signifies diffusion weighted images.");

      mitk::PropertyPersistenceInfo::Pointer PPI_referenceBValue = mitk::PropertyPersistenceInfo::New();
      PPI_referenceBValue->SetNameAndKey(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME, "DWMRI_b-value");
      mitk::PropertyPersistenceInfo::Pointer PPI_measurementFrame = mitk::PropertyPersistenceInfo::New();
      PPI_measurementFrame->SetNameAndKey(mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME, "measurement frame");
      mitk::PropertyPersistenceInfo::Pointer PPI_gradientContainer = mitk::PropertyPersistenceInfo::New();
      PPI_gradientContainer->SetNameAndKey(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME, "DWMRI_gradient");
      mitk::PropertyPersistenceInfo::Pointer PPI_modality = mitk::PropertyPersistenceInfo::New();
      PPI_modality->SetNameAndKey(mitk::DiffusionPropertyHelper::MODALITY, "modality");

      mitk::CoreServices::GetPropertyPersistence()->AddInfo(PPI_referenceBValue.GetPointer() , true);
      mitk::CoreServices::GetPropertyPersistence()->AddInfo(PPI_measurementFrame.GetPointer(), true);
      mitk::CoreServices::GetPropertyPersistence()->AddInfo(PPI_gradientContainer.GetPointer(), true);
      mitk::CoreServices::GetPropertyPersistence()->AddInfo(PPI_modality.GetPointer(), true);
    }

    void Unload(us::ModuleContext*) override
    {
      for (unsigned int loop(0); loop < m_MimeTypes.size(); ++loop)
      {
        delete m_MimeTypes.at(loop);
      }

      delete m_DiffusionImageNrrdReaderService;
      delete m_DiffusionImageNiftiReaderService;
      delete m_DiffusionImageFslNiftiReaderService;
      delete m_DiffusionImageDicomReaderService;
      delete m_NrrdTensorImageReader;
      delete m_NrrdOdfImageReader;
      delete m_PeakImageReader;
      delete m_ShImageReader;

      delete m_DiffusionImageNrrdWriterService;
      delete m_DiffusionImageNiftiWriterService;
      delete m_NrrdTensorImageWriter;
      delete m_NrrdOdfImageWriter;
      delete m_ShImageWriter;
    }

  private:

    DiffusionImageNrrdReaderService * m_DiffusionImageNrrdReaderService;
    DiffusionImageNiftiReaderService * m_DiffusionImageNiftiReaderService;
    DiffusionImageNiftiReaderService * m_DiffusionImageFslNiftiReaderService;
    DiffusionImageDicomReaderService * m_DiffusionImageDicomReaderService;
    NrrdTensorImageReader * m_NrrdTensorImageReader;
    NrrdOdfImageReader * m_NrrdOdfImageReader;
    PeakImageReader * m_PeakImageReader;
    ShImageReader * m_ShImageReader;

    DiffusionImageNrrdWriterService * m_DiffusionImageNrrdWriterService;
    DiffusionImageNiftiWriterService * m_DiffusionImageNiftiWriterService;
    NrrdTensorImageWriter * m_NrrdTensorImageWriter;
    NrrdOdfImageWriter * m_NrrdOdfImageWriter;
    ShImageWriter * m_ShImageWriter;

    std::vector<mitk::CustomMimeType*> m_MimeTypes;

  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::DiffusionCoreIOActivator)
