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

#include <mitkNrrdDiffusionImageReader.h>
#include <mitkNrrdTensorImageReader.h>

#include <mitkNrrdTensorImageWriter.h>

namespace mitk
{
  /**
  \brief Registers services for segmentation module.
  */
  class DiffusionModuleActivator : public us::ModuleActivator
  {
  public:

    void Load(us::ModuleContext* /*context*/)
    {
        m_NrrdDiffusionImageReader = new NrrdDiffusionImageReader();
        m_NrrdTensorImageReader = new NrrdTensorImageReader();

        m_NrrdTensorImageWriter = new NrrdTensorImageWriter();
    }

    void Unload(us::ModuleContext*)
    {
      delete m_NrrdDiffusionImageReader;
      delete m_NrrdTensorImageReader;

      delete m_NrrdTensorImageWriter;
    }

  private:

    NrrdDiffusionImageReader * m_NrrdDiffusionImageReader;
    NrrdTensorImageReader * m_NrrdTensorImageReader;

    NrrdTensorImageWriter * m_NrrdTensorImageWriter;

  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::DiffusionModuleActivator)
