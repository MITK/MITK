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

#ifndef COREEXTOBJECTFACTORY_H_INCLUDED
#define COREEXTOBJECTFACTORY_H_INCLUDED

#include "mitkCoreObjectFactoryBase.h"

namespace mitk {

class IOExtObjectFactory : public CoreObjectFactoryBase
{

public:

  mitkClassMacro(IOExtObjectFactory,CoreObjectFactoryBase)
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId) override;
  virtual void SetDefaultProperties(mitk::DataNode* node) override;
  virtual const char* GetFileExtensions() override;
  virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;
  virtual const char* GetSaveFileExtensions() override;
  virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;

private:

  IOExtObjectFactory();
  ~IOExtObjectFactory();

  void CreateFileExtensionsMap();
  MultimapType m_FileExtensionsMap;
  MultimapType m_SaveFileExtensionsMap;

  itk::ObjectFactoryBase::Pointer m_ParRecFileIOFactory;
  itk::ObjectFactoryBase::Pointer m_VtkUnstructuredGridIOFactory;
  itk::ObjectFactoryBase::Pointer m_StlVolumeTimeSeriesIOFactory;
  itk::ObjectFactoryBase::Pointer m_VtkVolumeTimeSeriesIOFactory;

  itk::ObjectFactoryBase::Pointer m_UnstructuredGridVtkWriterFactory;
};

}

#endif
