/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-18 16:49:06 +0200 (Mo, 18 Mai 2009) $
Version:   $Revision: 13129 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef DiffusionImagingObjectFactory_H_INCLUDED
#define DiffusionImagingObjectFactory_H_INCLUDED

#include "mitkCoreObjectFactory.h"
#include "MitkDiffusionImagingExports.h"

namespace mitk {

class MitkDiffusionImaging_EXPORT DiffusionImagingObjectFactory : public CoreObjectFactory
{
  public:
    mitkClassMacro(DiffusionImagingObjectFactory,CoreObjectFactory);
    itkNewMacro(DiffusionImagingObjectFactory);
    virtual Mapper::Pointer CreateMapper(mitk::DataTreeNode* node, MapperSlotId slotId);
    virtual void SetDefaultProperties(mitk::DataTreeNode* node);
    virtual const char* GetFileExtensions();
    virtual const char* GetSaveFileExtensions();
    virtual itk::Object::Pointer CreateCoreObject( const std::string& className );
    void RegisterIOFactories();
  protected:
    DiffusionImagingObjectFactory(bool registerSelf = true); 
  private:
    std::string m_ExternalFileExtensions;
    std::string m_InternalFileExtensions;
    std::string m_SaveFileExtensions;
};

}
// global declaration for simple call by
// applications
void MitkDiffusionImaging_EXPORT RegisterDiffusionImagingObjectFactory();

#endif

