/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITK_TOOLMANAGERPROVIDER_H
#define MITK_TOOLMANAGERPROVIDER_H

#include <MitkSegmentationExports.h>

#include <mitkServiceInterface.h>
#include <usModuleContext.h>

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
  class MITKSEGMENTATION_EXPORT ToolManagerProvider : public itk::LightObject
  {
  public:
    mitkClassMacroItkParent(ToolManagerProvider, itk::LightObject);

    /**
    \brief Returns ToolManager object.
    \note As this service is implemented as a singleton there is always the same ToolManager instance returned.
    */
    virtual mitk::ToolManager *GetToolManager();

    /**
    \brief Returns an instance of ToolManagerProvider service.
    */
    static mitk::ToolManagerProvider *GetInstance();

    // ONLY SegmentationModuleActivator is able to create instances of the service.
    friend class mitk::SegmentationModuleActivator;

  protected:
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      // hide everything
      ToolManagerProvider();
    ~ToolManagerProvider() override;
    ToolManagerProvider(const ToolManagerProvider &);
    ToolManagerProvider &operator=(const ToolManagerProvider &);

    mitk::ToolManager::Pointer m_ToolManager;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::ToolManagerProvider, "org.mitk.services.ToolManagerProvider")

#endif
