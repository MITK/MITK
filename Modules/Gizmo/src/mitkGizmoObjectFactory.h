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
  /**
   * \brief Registers mappers and assigns default properties for mitk::Gizmo.
   *
   * This class is still required by MITK to register custom mappers. Micro-service
   * registration for IO happens in GizmoActivator.
   */
  class MITKGIZMO_EXPORT GizmoObjectFactory : public mitk::CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(GizmoObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Create a mapper for the given node if it contains a Gizmo.
     * \param node The data node to create a mapper for.
     * \param slotId The mapper slot (2D or 3D).
     * \return A mapper instance, or nullptr if not applicable.
     */
    mitk::Mapper::Pointer CreateMapper(mitk::DataNode *node, MapperSlotId slotId) override;

    /**
     * \brief Assign default rendering properties for Gizmo nodes.
     * \param node The data node to set default properties on.
     */
    void SetDefaultProperties(mitk::DataNode *node) override;
  };
}

#endif
