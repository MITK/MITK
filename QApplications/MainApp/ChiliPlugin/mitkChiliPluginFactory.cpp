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

#include "mitkChiliPluginFactory.h"
#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include "mitkPACSPlugin.h"
#include "mitkChiliPlugin.h"
//TODO entfernen wenn das neue Chili-Release installiert ist
#include "mitkLightBoxImageReader.h"
#include "mitkLightBoxImageReaderImpl.h"


namespace mitk
{

template <class T>
class CreateChiliObjectFunction : public itk::CreateObjectFunctionBase
{
public:
  /** Standard class typedefs. */
  typedef CreateChiliObjectFunction  Self;
  typedef itk::SmartPointer<Self>    Pointer;

  /** Methods from itk:LightObject. */
  itkFactorylessNewMacro(Self);
  LightObject::Pointer CreateObject() { typename T::Pointer p = T::New(); 
    p->Register();
    return p.GetPointer(); 
  }

protected:
  CreateChiliObjectFunction() {}
  ~CreateChiliObjectFunction() {}

private:
  CreateChiliObjectFunction(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};


ChiliPluginFactory::ChiliPluginFactory()
{
  std::cout << "registering ChiliPluginFactory" << std::endl;

  this->RegisterOverride(typeid(mitk::PACSPlugin).name(),
                         typeid(mitk::ChiliPlugin).name(),
                         "MITK Chili Plugin",
                         1,
                         mitk::CreateChiliObjectFunction<mitk::ChiliPlugin>::New());

//TODO entfernen wenn das neue Chili-Release installiert ist
  this->RegisterOverride(typeid(mitk::LightBoxImageReader).name(),
                         typeid(mitk::LightBoxImageReaderImpl).name(),
                         "MITK Chili Plugin Lightbox Reader",
                         1,
                         mitk::CreateChiliObjectFunction<mitk::LightBoxImageReaderImpl>::New());
}

ChiliPluginFactory::~ChiliPluginFactory()
{
}

const char* ChiliPluginFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* ChiliPluginFactory::GetDescription() const
{
  return "Chili Plugin Factory, overrides stubs";
}

} // end namespace mitk

