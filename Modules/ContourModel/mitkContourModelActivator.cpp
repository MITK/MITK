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

#include "mitkContourModelSetReader.h"
#include "mitkContourModelReader.h"
#include "mitkContourModelWriter.h"
#include "mitkContourModelSetWriter.h"

#include <usModuleActivator.h>
#include <usModuleContext.h>


namespace mitk {

/*
 * This is the module activator for the "ContourModel" module.
 */
class ContourModelActivator : public us::ModuleActivator
{
public:

  void Load(us::ModuleContext* context)
  {
    m_ContourModelReader = new ContourModelReader();
    m_ContourModelSetReader = new ContourModelSetReader();
    m_ContourModelWriter = new ContourModelWriter();
    m_ContourModelSetWriter = new ContourModelSetWriter();
  }

  void Unload(us::ModuleContext* )
  {
    delete m_ContourModelReader;
    delete m_ContourModelSetReader;
    delete m_ContourModelWriter;
    delete m_ContourModelSetWriter;
  }

private:

  mitk::ContourModelReader* m_ContourModelReader;
  mitk::ContourModelSetReader* m_ContourModelSetReader;
  mitk::ContourModelWriter* m_ContourModelWriter;
  mitk::ContourModelSetWriter* m_ContourModelSetWriter;
};
}

US_EXPORT_MODULE_ACTIVATOR(mitk::ContourModelActivator)