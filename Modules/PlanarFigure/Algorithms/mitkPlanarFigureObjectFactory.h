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

#ifndef PLANARFIGUREOBJECTFACTORY_H_INCLUDED
#define PLANARFIGUREOBJECTFACTORY_H_INCLUDED

#include "mitkCoreObjectFactoryBase.h"
#include <MitkPlanarFigureExports.h>

namespace mitk {

class MitkPlanarFigure_EXPORT PlanarFigureObjectFactory : public CoreObjectFactoryBase
{
public:

  mitkClassMacro(PlanarFigureObjectFactory,CoreObjectFactoryBase)
  itkNewMacro(PlanarFigureObjectFactory)

  ~PlanarFigureObjectFactory();

  virtual Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId);

  virtual void SetDefaultProperties(mitk::DataNode* node);

  virtual const char* GetFileExtensions();

  virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap();

  virtual const char* GetSaveFileExtensions();

  virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap();

  DEPRECATED(void RegisterIOFactories());

protected:
  PlanarFigureObjectFactory();
  void CreateFileExtensionsMap();
  MultimapType m_FileExtensionsMap;
  MultimapType m_SaveFileExtensionsMap;

private:

  itk::ObjectFactoryBase::Pointer m_PlanarFigureIOFactory;
  itk::ObjectFactoryBase::Pointer m_PlanarFigureWriterFactory;
};

}

#endif // PLANARFIGUREOBJECTFACTORY_H_INCLUDED
