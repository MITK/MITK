/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGizmoObjectFactory_h
#define mitkGizmoObjectFactory_h

// MITK includes
#include <mitkCoreObjectFactory.h>

// Export definitions
#include <MitkGizmoExports.h>

namespace mitk
{
  /*
      \brief Registers mappers and assigns default properties for mitk::Gizmo.

      This class is still required by MITK to register custom mappers.
      The base class interface requires also methods that describe file
      reader/writer extensions, but this is already deprecated. We do
      not implement those methods but use the new micro-service registration
      via GizmoActivator.
  */
  class MITKGIZMO_EXPORT GizmoObjectFactory : public mitk::CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(GizmoObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    // Create a mapper for given node
    mitk::Mapper::Pointer CreateMapper(mitk::DataNode *node, MapperSlotId slotId) override;

    // Assign default properties to given node
    void SetDefaultProperties(mitk::DataNode *node) override;

    // Deprecated but required!
    std::string GetFileExtensions() override { return ""; }
    // Deprecated but required!
    mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override
    {
      return CoreObjectFactoryBase::MultimapType();
    }

    // Deprecated but required!
    std::string GetSaveFileExtensions() override { return ""; }
    // Deprecated but required!
    mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override
    {
      return CoreObjectFactoryBase::MultimapType();
    }
  };
}

#endif
