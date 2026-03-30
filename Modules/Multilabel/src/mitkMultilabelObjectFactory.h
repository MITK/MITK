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

    /** \brief Return supported file read extensions as a string. */
    std::string GetFileExtensions() override;

    /** \brief Return a map of file read extensions to descriptions. */
    mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;

    /** \brief Return supported file write extensions as a string. */
    std::string GetSaveFileExtensions() override;

    /** \brief Return a map of file write extensions to descriptions. */
    mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;

  protected:
    MultilabelObjectFactory();
    ~MultilabelObjectFactory() override;
    void CreateFileExtensionsMap();
  };
}

#endif
