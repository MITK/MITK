/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourModelReader.h"
#include "mitkContourModelSetReader.h"
#include "mitkContourModelSetWriter.h"
#include "mitkContourModelWriter.h"

#include <usModuleActivator.h>
#include <usModuleContext.h>

namespace mitk
{
  /*
   * This is the module activator for the "ContourModel" module.
   */
  class ContourModelActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext * /*context*/) override
    {
      m_ContourModelReader = new ContourModelReader();
      m_ContourModelSetReader = new ContourModelSetReader();
      m_ContourModelWriter = new ContourModelWriter();
      m_ContourModelSetWriter = new ContourModelSetWriter();
    }

    void Unload(us::ModuleContext *) override
    {
      delete m_ContourModelReader;
      delete m_ContourModelSetReader;
      delete m_ContourModelWriter;
      delete m_ContourModelSetWriter;
    }

  private:
    mitk::ContourModelReader *m_ContourModelReader;
    mitk::ContourModelSetReader *m_ContourModelSetReader;
    mitk::ContourModelWriter *m_ContourModelWriter;
    mitk::ContourModelSetWriter *m_ContourModelSetWriter;
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::ContourModelActivator)
