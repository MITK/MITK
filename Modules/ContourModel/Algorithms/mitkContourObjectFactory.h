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

#ifndef SEGMENTATIONOBJECTFACTORY_H_INCLUDED
#define SEGMENTATIONOBJECTFACTORY_H_INCLUDED

#include "mitkCoreObjectFactoryBase.h"
#include <MitkContourModelExports.h>

namespace mitk {

  class MITKCONTOURMODEL_EXPORT ContourObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(ContourObjectFactory,CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    virtual Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId) override;
    virtual void SetDefaultProperties(mitk::DataNode* node) override;
    virtual const char* GetFileExtensions() override;
    virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;
    virtual const char* GetSaveFileExtensions() override;
    virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;

    /** \deprecatedSince{2013_09} */
    DEPRECATED(void RegisterIOFactories());

  protected:
    ContourObjectFactory();
    ~ContourObjectFactory();
    void CreateFileExtensionsMap();
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;

  private:

  };

}

#endif
