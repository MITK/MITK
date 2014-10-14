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

  class MitkContourModel_EXPORT ContourObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(ContourObjectFactory,CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    virtual Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId);
    virtual void SetDefaultProperties(mitk::DataNode* node);
    virtual const char* GetFileExtensions();
    virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap();
    virtual const char* GetSaveFileExtensions();
    virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap();

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
