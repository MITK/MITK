/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef COREOBJECTFACTORY_H_INCLUDED
#define COREOBJECTFACTORY_H_INCLUDED

#include <list>

#include "mitkCommon.h"
#include "mitkCoreObjectFactoryBase.h"
#include "mitkFileWriterWithInformation.h"
namespace mitk {

class Event;

class MITK_CORE_EXPORT CoreObjectFactory : public CoreObjectFactoryBase
{
  public:
    mitkClassMacro(CoreObjectFactory,CoreObjectFactoryBase);
    itkFactorylessNewMacro(CoreObjectFactory);
    virtual Mapper::Pointer CreateMapper(mitk::DataTreeNode* node, MapperSlotId slotId);
    virtual void SetDefaultProperties(mitk::DataTreeNode* node);
    virtual const char* GetFileExtensions();
    virtual const char* GetSaveFileExtensions();
    virtual FileWriterList GetFileWriters();
    virtual itk::Object::Pointer CreateCoreObject( const std::string& className );
    virtual void MapEvent(const mitk::Event* event, const int eventID);
    virtual void RegisterExtraFactory(CoreObjectFactoryBase::Pointer factory);
    static Pointer GetInstance();
  protected:
    CoreObjectFactory(); 
    typedef std::list<mitk::CoreObjectFactoryBase::Pointer> ExtraFactoriesList;
    ExtraFactoriesList m_ExtraFactories;
    static FileWriterList m_FileWriters;
    std::string m_FileExtensions;
    std::string m_SaveFileExtensions;

};

} // namespace mitk

#endif 
