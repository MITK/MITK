/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 13129 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef COREEXTOBJECTFACTORY_H_INCLUDED
#define COREEXTOBJECTFACTORY_H_INCLUDED

#include "mitkCoreObjectFactory.h"

namespace mitk {

class MITKEXT_CORE_EXPORT CoreExtObjectFactory : public CoreObjectFactory
{
  public:
    mitkClassMacro(CoreExtObjectFactory,CoreObjectFactory);
    itkNewMacro(CoreExtObjectFactory);
    virtual Mapper::Pointer CreateMapper(mitk::DataTreeNode* node, MapperSlotId slotId);
    virtual void SetDefaultProperties(mitk::DataTreeNode* node);
    virtual const char* GetFileExtensions();
    virtual const char* GetSaveFileExtensions();
    virtual itk::Object::Pointer CreateCoreObject( const std::string& className );
    void RegisterIOFactories();
  protected:
    CoreExtObjectFactory(bool registerSelf = true); 
};

}
// global declaration for simple call by
// applications
void MITKEXT_CORE_EXPORT RegisterCoreExtObjectFactory();

#endif

