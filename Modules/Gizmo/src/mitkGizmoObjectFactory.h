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

#ifndef GizmoObjectFactor_h
#define GizmoObjectFactor_h

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
class  MITKGIZMO_EXPORT GizmoObjectFactory : public mitk::CoreObjectFactoryBase
{

public:
    mitkClassMacro(GizmoObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    // Create a mapper for given node
    virtual mitk::Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId) override;

    // Assign default properties to given node
    virtual void SetDefaultProperties(mitk::DataNode* node) override;

    // Deprecated but required!
    virtual const char* GetFileExtensions() override
    {
        return "";
    }

    // Deprecated but required!
    virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override
    {
        return CoreObjectFactoryBase::MultimapType();
    }

    // Deprecated but required!
    virtual const char* GetSaveFileExtensions() override
    {
        return "";
    }

    // Deprecated but required!
    virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override
    {
        return CoreObjectFactoryBase::MultimapType();
    }
};

}

#endif
