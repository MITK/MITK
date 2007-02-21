/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkCommon.h"
#include "mitkCoreObjectFactoryBase.h"

namespace mitk {

class CoreObjectFactory : public CoreObjectFactoryBase
{
  public:
    static Mapper::Pointer CreateMapper(mitk::DataTreeNode* node, MapperSlotId slotId);
    static itk::Object::Pointer CreateCoreObject( const std::string& className );
    mitkClassMacro(CoreObjectFactory,CoreObjectFactoryBase);
    itkNewMacro(CoreObjectFactory);
    virtual Mapper::Pointer CreateSpecialMapper(mitk::DataTreeNode* node, MapperSlotId slotId);
    virtual itk::Object::Pointer CreateSpecialCoreObject( const std::string& className );
  protected:
    CoreObjectFactory(); 
    static void RegisterSelfOnce();
};

} // namespace mitk

#endif /* MAPPERFACTORY_H_HEADER_INCLUDED_C179C58F */
