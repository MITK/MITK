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

#include <mitkFiberBundleXReader.h>
#include <mitkFiberBundleXWriter.h>

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
        m_FiberBundleXReader = new FiberBundleXReader();
        m_FiberBundleXWriter = new FiberBundleXWriter();
    }

    void Unload(us::ModuleContext*)
    {
      delete m_FiberBundleXReader;
      delete m_FiberBundleXWriter;
    }

  private:

    FiberBundleXReader * m_FiberBundleXReader;
    FiberBundleXWriter * m_FiberBundleXWriter;

  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::DiffusionModuleActivator)
