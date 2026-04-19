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
   * Registers mappers, default properties, and legacy IO factories for
   * extended data types such as UnstructuredGrid and various volume time series.
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

    itk::ObjectFactoryBase::Pointer m_ParRecFileIOFactory;
    itk::ObjectFactoryBase::Pointer m_VtkUnstructuredGridIOFactory;
    itk::ObjectFactoryBase::Pointer m_StlVolumeTimeSeriesIOFactory;
    itk::ObjectFactoryBase::Pointer m_VtkVolumeTimeSeriesIOFactory;

    itk::ObjectFactoryBase::Pointer m_UnstructuredGridVtkWriterFactory;
  };
}

#endif
