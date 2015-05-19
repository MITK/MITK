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
#include <mitkFiberBundleVtkReader.h>
#include <mitkFiberBundleTrackVisReader.h>
#include <mitkConnectomicsNetworkReader.h>
#include <mitkPlanarFigureCompositeReader.h>

#include <mitkDiffusionImageNrrdWriterService.h>
#include <mitkDiffusionImageNiftiWriterService.h>
#include <mitkNrrdTensorImageWriter.h>
#include <mitkNrrdQBallImageWriter.h>
#include <mitkFiberBundleVtkWriter.h>
#include <mitkFiberBundleTrackVisWriter.h>
#include <mitkConnectomicsNetworkWriter.h>
#include <mitkPlanarFigureCompositeWriter.h>

#include "mitkDiffusionIOMimeTypes.h"

namespace mitk
{
  /**
  \brief Registers services for segmentation module.
  */
  class DiffusionModuleActivator : public us::ModuleActivator
  {
  public:

    void Load(us::ModuleContext* context) override
    {
      us::ServiceProperties props;
      props[ us::ServiceConstants::SERVICE_RANKING() ] = 10;

      m_MimeTypes = mitk::DiffusionIOMimeTypes::Get();
      for (std::vector<mitk::CustomMimeType*>::const_iterator mimeTypeIter = m_MimeTypes.begin(),
        iterEnd = m_MimeTypes.end(); mimeTypeIter != iterEnd; ++mimeTypeIter)
      {
        context->RegisterService(*mimeTypeIter, props);
      }

      m_DiffusionImageNrrdReaderService = new DiffusionImageNrrdReaderService();
      m_DiffusionImageNiftiReaderService = new DiffusionImageNiftiReaderService();
      m_NrrdTensorImageReader = new NrrdTensorImageReader();
      m_NrrdQBallImageReader = new NrrdQBallImageReader();
      m_FiberBundleVtkReader = new FiberBundleVtkReader();
      m_FiberBundleTrackVisReader = new FiberBundleTrackVisReader();
      m_ConnectomicsNetworkReader = new ConnectomicsNetworkReader();
      m_PlanarFigureCompositeReader = new PlanarFigureCompositeReader();

      m_DiffusionImageNrrdWriterService = new DiffusionImageNrrdWriterService();
      m_DiffusionImageNiftiWriterService = new DiffusionImageNiftiWriterService();
      m_NrrdTensorImageWriter = new NrrdTensorImageWriter();
      m_NrrdQBallImageWriter = new NrrdQBallImageWriter();
      m_FiberBundleVtkWriter = new FiberBundleVtkWriter();
      m_FiberBundleTrackVisWriter = new FiberBundleTrackVisWriter();
      m_ConnectomicsNetworkWriter = new ConnectomicsNetworkWriter();
      m_PlanarFigureCompositeWriter = new PlanarFigureCompositeWriter();
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
      delete m_FiberBundleVtkReader;
      delete m_FiberBundleTrackVisReader;
      delete m_ConnectomicsNetworkReader;
      delete m_PlanarFigureCompositeReader;

      delete m_DiffusionImageNrrdWriterService;
      delete m_DiffusionImageNiftiWriterService;
      delete m_NrrdTensorImageWriter;
      delete m_NrrdQBallImageWriter;
      delete m_FiberBundleVtkWriter;
      delete m_FiberBundleTrackVisWriter;
      delete m_ConnectomicsNetworkWriter;
      delete m_PlanarFigureCompositeWriter;
    }

  private:

    DiffusionImageNrrdReaderService * m_DiffusionImageNrrdReaderService;
    DiffusionImageNiftiReaderService * m_DiffusionImageNiftiReaderService;
    NrrdTensorImageReader * m_NrrdTensorImageReader;
    NrrdQBallImageReader * m_NrrdQBallImageReader;
    FiberBundleVtkReader * m_FiberBundleVtkReader;
    FiberBundleTrackVisReader * m_FiberBundleTrackVisReader;
    ConnectomicsNetworkReader * m_ConnectomicsNetworkReader;
    PlanarFigureCompositeReader* m_PlanarFigureCompositeReader;

    DiffusionImageNrrdWriterService * m_DiffusionImageNrrdWriterService;
    DiffusionImageNiftiWriterService * m_DiffusionImageNiftiWriterService;
    NrrdTensorImageWriter * m_NrrdTensorImageWriter;
    NrrdQBallImageWriter * m_NrrdQBallImageWriter;
    FiberBundleVtkWriter * m_FiberBundleVtkWriter;
    FiberBundleTrackVisWriter * m_FiberBundleTrackVisWriter;
    ConnectomicsNetworkWriter * m_ConnectomicsNetworkWriter;
    PlanarFigureCompositeWriter* m_PlanarFigureCompositeWriter;

    std::vector<mitk::CustomMimeType*> m_MimeTypes;

  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::DiffusionModuleActivator)
