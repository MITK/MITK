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

#include <mitkFiberBundleVtkReader.h>
#include <mitkFiberBundleTckReader.h>
#include <mitkFiberBundleTrackVisReader.h>
#include <mitkConnectomicsNetworkReader.h>
#include <mitkPlanarFigureCompositeReader.h>

#include <mitkFiberBundleVtkWriter.h>
#include <mitkFiberBundleTrackVisWriter.h>
#include <mitkConnectomicsNetworkWriter.h>
#include <mitkConnectomicsNetworkCSVWriter.h>
#include <mitkConnectomicsNetworkMatrixWriter.h>
#include <mitkPlanarFigureCompositeWriter.h>

#include <mitkCoreServices.h>
#include <mitkIPropertyDescriptions.h>
#include <mitkIPropertyPersistence.h>

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

      m_FiberBundleVtkReader = new FiberBundleVtkReader();
      m_FiberBundleTrackVisReader = new FiberBundleTrackVisReader();
      m_FiberBundleTckReader = new FiberBundleTckReader();
      m_ConnectomicsNetworkReader = new ConnectomicsNetworkReader();
      m_PlanarFigureCompositeReader = new PlanarFigureCompositeReader();

      m_FiberBundleVtkWriter = new FiberBundleVtkWriter();
      m_FiberBundleTrackVisWriter = new FiberBundleTrackVisWriter();
      m_ConnectomicsNetworkWriter = new ConnectomicsNetworkWriter();
      m_ConnectomicsNetworkCSVWriter = new ConnectomicsNetworkCSVWriter();
      m_ConnectomicsNetworkMatrixWriter = new ConnectomicsNetworkMatrixWriter();
      m_PlanarFigureCompositeWriter = new PlanarFigureCompositeWriter();
    }

    void Unload(us::ModuleContext*) override
    {
      for (unsigned int loop(0); loop < m_MimeTypes.size(); ++loop)
      {
        delete m_MimeTypes.at(loop);
      }

      delete m_FiberBundleVtkReader;
      delete m_FiberBundleTckReader;
      delete m_FiberBundleTrackVisReader;
      delete m_ConnectomicsNetworkReader;
      delete m_PlanarFigureCompositeReader;

      delete m_FiberBundleVtkWriter;
      delete m_FiberBundleTrackVisWriter;
      delete m_ConnectomicsNetworkWriter;
      delete m_ConnectomicsNetworkCSVWriter;
      delete m_ConnectomicsNetworkMatrixWriter;
      delete m_PlanarFigureCompositeWriter;
    }

  private:

    FiberBundleVtkReader * m_FiberBundleVtkReader;
    FiberBundleTckReader * m_FiberBundleTckReader;
    FiberBundleTrackVisReader * m_FiberBundleTrackVisReader;
    ConnectomicsNetworkReader * m_ConnectomicsNetworkReader;
    PlanarFigureCompositeReader* m_PlanarFigureCompositeReader;

    FiberBundleVtkWriter * m_FiberBundleVtkWriter;
    FiberBundleTrackVisWriter * m_FiberBundleTrackVisWriter;
    ConnectomicsNetworkWriter * m_ConnectomicsNetworkWriter;
    ConnectomicsNetworkCSVWriter * m_ConnectomicsNetworkCSVWriter;
    ConnectomicsNetworkMatrixWriter * m_ConnectomicsNetworkMatrixWriter;
    PlanarFigureCompositeWriter* m_PlanarFigureCompositeWriter;

    std::vector<mitk::CustomMimeType*> m_MimeTypes;

  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::DiffusionModuleActivator)
