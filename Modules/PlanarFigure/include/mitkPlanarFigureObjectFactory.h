/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef PLANARFIGUREOBJECTFACTORY_H_INCLUDED
#define PLANARFIGUREOBJECTFACTORY_H_INCLUDED

#include "mitkCoreObjectFactoryBase.h"
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  class MITKPLANARFIGURE_EXPORT PlanarFigureObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(PlanarFigureObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      ~PlanarFigureObjectFactory() override;

    Mapper::Pointer CreateMapper(mitk::DataNode *node, MapperSlotId slotId) override;

    void SetDefaultProperties(mitk::DataNode *node) override;

    std::string GetFileExtensions() override;

    mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;

    std::string GetSaveFileExtensions() override;

    mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;

  protected:
    PlanarFigureObjectFactory();
    void CreateFileExtensionsMap();
  };
}

#endif // PLANARFIGUREOBJECTFACTORY_H_INCLUDED
