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

#include <string>
#include <map>

namespace mitk
{
  class SegmentationModuleActivator;

  /**
   \brief Micro Service Singleton to get an instance of mitk::ToolManager

   \sa ToolManager

   Implemented as a registry to have implicitly only one instance of ToolManager per context.
   Context is nothing more than a unique string, for example, use your plugin's id as context.
   Use this service to make sure your Tools are managed by the object.

   \note Can only be instantiated by SegmentationModuleActivator. The common way to get the ToolManager is by
   <code> mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager("my context"); </code>
  */
  class MITKSEGMENTATION_EXPORT ToolManagerProvider : public itk::LightObject
  {
  public:
    mitkClassMacroItkParent(ToolManagerProvider, itk::LightObject);

    using ProviderMapType = std::map<std::string, mitk::ToolManager::Pointer>;

    // Known ToolManager contexts
    static const char* const SEGMENTATION;
    static const char* const MULTILABEL_SEGMENTATION;

    /**
    \brief Returns ToolManager object.
    \param context A unique, non-empty string to retrieve a certain ToolManager instance. For backwards-compatibility, the overload without
    arguments is returning the classic segmentation ToolManager instance.
    If a ToolManager instance does not yet exist for a given context, it is created on the fly.
    */
    virtual mitk::ToolManager *GetToolManager(const std::string& context = SEGMENTATION);

    /**
    \brief Returns all registered ToolManager objects.
    */
    ProviderMapType GetToolManagers() const;

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

    ProviderMapType m_ToolManagers;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::ToolManagerProvider, "org.mitk.services.ToolManagerProvider")

#endif
