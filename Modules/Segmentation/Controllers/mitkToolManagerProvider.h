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

#include <MitkSegmentationExports.h>

#include <usModuleContext.h>
#include <mitkServiceInterface.h>

#include "mitkToolManager.h"
#include <itkObjectFactory.h>
#include <mitkCommon.h>



namespace mitk
{

  class SegmentationModuleActivator;

  /**
   \brief Micro Service Singleton to get an instance of mitk::ToolManager

   \sa ToolManager

   Implemented as a singleton to have implicitly only one instance of ToolManager.
   Use this service to make sure your Tools are managed by the object.

   \note Can only be instantiated by SegmentationModuleActivator. The common way to get the ToolManager is by
   <code> mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager(); </code>
  */
  class MitkSegmentation_EXPORT ToolManagerProvider : public itk::LightObject
  {
  public:
    mitkClassMacro(ToolManagerProvider, itk::LightObject);

    /**
    \brief Returns ToolManager object.
    \note As this service is implemented as a singleton there is always the same ToolManager instance returned.
    */
    virtual mitk::ToolManager* GetToolManager();

    /**
    \brief Returns an instance of ToolManagerProvider service.
    */
    static mitk::ToolManagerProvider* GetInstance();

    //ONLY SegmentationModuleActivator is able to create instances of the service.
    friend class mitk::SegmentationModuleActivator;

  protected:
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    //hide everything
    ToolManagerProvider();
    virtual ~ToolManagerProvider();
    ToolManagerProvider(const ToolManagerProvider&);
    ToolManagerProvider& operator=(const ToolManagerProvider&);

    mitk::ToolManager::Pointer m_ToolManager;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::ToolManagerProvider, "org.mitk.services.ToolManagerProvider")

#endif
