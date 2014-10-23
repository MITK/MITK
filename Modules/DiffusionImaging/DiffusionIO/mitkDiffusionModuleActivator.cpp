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

#include <mitkNrrdDiffusionImageReader.h>
#include <mitkNrrdTensorImageReader.h>
#include <mitkNrrdQBallImageReader.h>
#include <mitkFiberBundleXReader.h>

#include <mitkNrrdTensorImageWriter.h>
#include <mitkNrrdQBallImageWriter.h>
#include <mitkFiberBundleXWriter.h>

#include "mitkDiffusionIOMimeTypes.h"

namespace mitk
{
  /**
  \brief Registers services for segmentation module.
  */
  class DiffusionModuleActivator : public us::ModuleActivator
  {
  public:

    void Load(us::ModuleContext* context)
    {
      us::ServiceProperties props;
      props[ us::ServiceConstants::SERVICE_RANKING() ] = 10;

      std::vector<mitk::CustomMimeType*> mimeTypes = mitk::DiffusionIOMimeTypes::Get();
      for (std::vector<mitk::CustomMimeType*>::const_iterator mimeTypeIter = mimeTypes.begin(),
        iterEnd = mimeTypes.end(); mimeTypeIter != iterEnd; ++mimeTypeIter)
      {
        context->RegisterService(*mimeTypeIter, props);
      }

      m_NrrdDiffusionImageReader = new NrrdDiffusionImageReader();
      m_NrrdTensorImageReader = new NrrdTensorImageReader();
      m_NrrdQBallImageReader = new NrrdQBallImageReader();
      m_FiberBundleXReader = new FiberBundleXReader();

      m_NrrdTensorImageWriter = new NrrdTensorImageWriter();
      m_NrrdQBallImageWriter = new NrrdQBallImageWriter();
      m_FiberBundleXWriter = new FiberBundleXWriter();
    }

    void Unload(us::ModuleContext*)
    {
      delete m_NrrdDiffusionImageReader;
      delete m_NrrdTensorImageReader;
      delete m_NrrdQBallImageReader;
      delete m_FiberBundleXReader;

      delete m_NrrdTensorImageWriter;
      delete m_NrrdQBallImageWriter;
      delete m_FiberBundleXWriter;
    }

  private:

    NrrdDiffusionImageReader * m_NrrdDiffusionImageReader;
    NrrdTensorImageReader * m_NrrdTensorImageReader;
    NrrdQBallImageReader * m_NrrdQBallImageReader;
    FiberBundleXReader * m_FiberBundleXReader;

    NrrdTensorImageWriter * m_NrrdTensorImageWriter;
    NrrdQBallImageWriter * m_NrrdQBallImageWriter;
    FiberBundleXWriter * m_FiberBundleXWriter;

  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::DiffusionModuleActivator)
