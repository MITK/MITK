/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIOExtObjectFactory_h
#define mitkIOExtObjectFactory_h

#include <mitkCoreObjectFactoryBase.h>

namespace mitk
{
  /**
   * \brief Object factory for extended IO types.
   *
   * Registers mappers and default rendering properties for extended data types
   * such as UnstructuredGrid.
   */
  class IOExtObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(IOExtObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Create a mapper for the given node and slot. */
    Mapper::Pointer CreateMapper(mitk::DataNode *node, MapperSlotId slotId) override;

    /** \brief Set default rendering properties on the given node. */
    void SetDefaultProperties(mitk::DataNode *node) override;

  private:
    IOExtObjectFactory();
    ~IOExtObjectFactory() override;
  };
}

#endif
