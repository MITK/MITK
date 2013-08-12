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

#include <usGetModuleContext.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usModule.h>

#include "mitkContourManagerImpl.h"

namespace mitk
{
  class SegmentationModuleActivator : public mitk::ModuleActivator
  {
  public:

    void Load(mitk::ModuleContext* context)
    {
    }

    void Unload(ModuleContext*)
    {
    }

  private:
    mitk::ContourManagerImpl::Pointer m_ContourManagerImpl;
  };
}

US_EXPORT_MODULE_ACTIVATOR(Segmentation, mitk::SegmentationModuleActivator)
