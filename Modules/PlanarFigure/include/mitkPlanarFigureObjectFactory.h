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

namespace mitk
{
  class MITKPLANARFIGURE_EXPORT PlanarFigureObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(PlanarFigureObjectFactory, CoreObjectFactoryBase) itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      ~PlanarFigureObjectFactory() override;

    Mapper::Pointer CreateMapper(mitk::DataNode *node, MapperSlotId slotId) override;

    void SetDefaultProperties(mitk::DataNode *node) override;

    const char *GetFileExtensions() override;

    mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;

    const char *GetSaveFileExtensions() override;

    mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;

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
