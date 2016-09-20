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

#include "mitkRTDoseReader.h"

#include <usModuleActivator.h>
#include <usModuleContext.h>


namespace mitk {

/*
 * This is the module activator for the "VirtuosIO" module.
 */
class DICOMRTIOActivator : public us::ModuleActivator
{
public:

  void Load(us::ModuleContext* context)
  {
    m_RTDoseReader = new RTDoseReader();
  }

  void Unload(us::ModuleContext* )
  {
    delete m_RTDoseReader;
  }

private:
  mitk::RTDoseReader* m_RTDoseReader;
};
}

US_EXPORT_MODULE_ACTIVATOR(mitk::DICOMRTIOActivator)