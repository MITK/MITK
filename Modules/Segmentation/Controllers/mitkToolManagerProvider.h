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

#ifndef MITK_TOOLMANAGERPROVIDER_H
#define MITK_TOOLMANAGERPROVIDER_H

#include <usModuleContext.h>
#include <usServiceInterface.h>

#include "mitkToolManager.h"
#include <itkObjectFactory.h>
#include <mitkCommon.h>

//class SegmentationModuleActivator;

namespace mitk
{

  class ToolManagerProvider : public itk::LightObject
  {
  public:
    mitkClassMacro(ToolManagerProvider, itk::LightObject);
    itkNewMacro(ToolManagerProvider);

    virtual mitk::ToolManager* GetToolManager();

    static mitk::ToolManagerProvider* GetInstance();

    //friend class SegmentationModuleActivator;

  protected:
    //mitkClassMacro(ToolManagerProvider, itk::LightObject);
    //itkNewMacro(Self);

    ToolManagerProvider();
    virtual ~ToolManagerProvider();

    mitk::ToolManager::Pointer m_ToolManager;
  };
}

US_DECLARE_SERVICE_INTERFACE(mitk::ToolManagerProvider, "org.mitk.services.ToolManagerProvider")

#endif
