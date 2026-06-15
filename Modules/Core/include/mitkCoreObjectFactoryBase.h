/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCoreObjectFactoryBase_h
#define mitkCoreObjectFactoryBase_h

// the mitkLog.h header is necessary for CMake test drivers.
// Since the EXTRA_INCLUDE parameter of CREATE_TEST_SOURCELIST only
// allows one extra include file, we specify mitkLog.h here so it will
// be available to all classes implementing this interface.
#include <mitkLog.h>

#include <mitkMapper.h>
#include <MitkCoreExports.h>
#include <itkObjectFactoryBase.h>
#include <itkVersion.h>

namespace mitk
{
  class DataNode;

  /**
   * \brief Abstract base class for factories that create mappers and set default properties.
   *
   * This interface can be implemented by factories that add new mapper classes
   * or extend the data tree deserialization mechanism. Module-specific factories
   * deriving from this class can be registered with CoreObjectFactory.
   *
   * \sa CoreObjectFactory Mapper DataNode
   */
  class MITKCORE_EXPORT CoreObjectFactoryBase : public itk::Object
  {
  public:
    mitkClassMacroItkParent(CoreObjectFactoryBase, itk::Object);

    /**
     * \brief Create a mapper for the given data node and mapper slot.
     * \param node   The data node to create a mapper for.
     * \param slotId The mapper slot (2D or 3D).
     * \return A new mapper, or nullptr if not supported.
     */
    virtual Mapper::Pointer CreateMapper(mitk::DataNode *node, MapperSlotId slotId) = 0;

    /**
     * \brief Set default rendering properties on the given data node.
     * \param node The data node to configure.
     */
    virtual void SetDefaultProperties(mitk::DataNode *node) = 0;

    /**
     * \brief Get the ITK source version string.
     * \return The ITK source version used to build MITK.
     */
    virtual const char *GetITKSourceVersion() const { return ITK_SOURCE_VERSION; }

    /**
     * \brief Get a human-readable description of this factory.
     * \return The description string.
     */
    virtual const char *GetDescription() const { return "Core Object Factory"; }
  };
}
#endif
