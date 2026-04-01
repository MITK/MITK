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

    /** \brief Return supported file read extensions. */
    std::string GetFileExtensions() override;

    /** \brief Return a map of file read extensions to descriptions. */
    MultimapType GetFileExtensionsMap() override;

    /** \brief Return supported file write extensions. */
    std::string GetSaveFileExtensions() override;

    /** \brief Return a map of file write extensions to descriptions. */
    MultimapType GetSaveFileExtensionsMap() override;

  protected:
    ROIObjectFactory();
    ~ROIObjectFactory() override;
  };
}

#endif
