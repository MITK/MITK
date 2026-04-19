/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultilabelObjectFactory_h
#define mitkMultilabelObjectFactory_h

#include <mitkCoreObjectFactoryBase.h>

#include <mitkAbstractFileIO.h>

namespace mitk
{
  /**
   * \brief Object factory that registers mappers and default properties for MultiLabelSegmentation.
   */
  class MultilabelObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(MultilabelObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self)

    /** \brief Create a mapper for the given node and slot. */
    Mapper::Pointer CreateMapper(mitk::DataNode *node, MapperSlotId slotId) override;

    /** \brief Set default rendering properties on the given node. */
    void SetDefaultProperties(mitk::DataNode *node) override;

  protected:
    MultilabelObjectFactory();
    ~MultilabelObjectFactory() override;
  };
}

#endif
