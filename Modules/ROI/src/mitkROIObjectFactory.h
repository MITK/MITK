/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkROIObjectFactory_h
#define mitkROIObjectFactory_h

#include <mitkCoreObjectFactoryBase.h>

namespace mitk
{
  /**
   * \brief Object factory that registers mappers and default properties for mitk::ROI.
   */
  class ROIObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(ROIObjectFactory, CoreObjectFactoryBase)
    itkFactorylessNewMacro(Self)

    /** \brief Create a mapper for the given node and slot. */
    Mapper::Pointer CreateMapper(DataNode* node, MapperSlotId slotId) override;

    /** \brief Set default rendering properties on the given node. */
    void SetDefaultProperties(DataNode *node) override;

  protected:
    ROIObjectFactory();
    ~ROIObjectFactory() override;
  };
}

#endif
