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

#ifndef MITK_TOOLMANAGERPROVIDER_IMPL_H
#define MITK_TOOLMANAGERPROVIDER_IMPL_H

#include <usModuleContext.h>

#include "mitkContourManager.h"

#include <itkObjectFactory.h>
#include <mitkCommon.h>

namespace mitk
{

  class ToolManagerProviderImpl : public itk::LightObject, public ToolManagerProvider
  {
  public:
    mitkClassMacro(ToolManagerProviderImpl, itk::LightObject);
    itkNewMacro(Self);

    virtual bool Dummy();
    //friend class mitk::SegmentationModuleActivator;

  protected:
    //mitkClassMacro(ToolManagerProviderImpl, itk::LightObject);
    //itkNewMacro(Self);

    ToolManagerProviderImpl();
    virtual ~ToolManagerProviderImpl();
  };
}

#endif
