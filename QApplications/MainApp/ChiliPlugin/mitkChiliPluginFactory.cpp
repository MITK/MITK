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

#include "mitkChiliPlugin.h"
#include "mitkChiliPluginImpl.h"
#include "mitkLightBoxImageReader.h"
#include "mitkLightBoxImageReaderImpl.h"
#include "mitkLightBoxResultImageWriter.h"
#include "mitkLightBoxResultImageWriterImpl.h"

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
  /*this->RegisterOverride("mitk::ChiliPlugin",
                         "mitk::ChiliPluginImpl",
                         "MITK Chili Plugin",
                         1,
                         itk::CreateObjectFunction<mitk::ChiliPluginImpl>::New());
  this->RegisterOverride("mitk::LightBoxImageReader",
                         "mitk::LightBoxImageReaderImpl",
                         "MITK Chili Plugin Lightbox Reader",
                         1,
                         itk::CreateObjectFunction<mitk::LightBoxImageReaderImpl>::New());
  this->RegisterOverride("mitk::LightBoxResultImageWriter",
                         "mitk::LightBoxResultImageWriterImpl",
                         "MITK Chili Plugin Lightbox Writer",
                         1,
                         mitk::CreateChiliObjectFunction<mitk::LightBoxResultImageWriterImpl>::New());*/
  this->RegisterOverride(typeid(mitk::ChiliPlugin).name(),
                         typeid(mitk::ChiliPluginImpl).name(),
                         "MITK Chili Plugin",
                         1,
                         mitk::CreateChiliObjectFunction<mitk::ChiliPluginImpl>::New());
  this->RegisterOverride(typeid(mitk::LightBoxImageReader).name(),
                         typeid(mitk::LightBoxImageReaderImpl).name(),
                         "MITK Chili Plugin Lightbox Reader).name()",
                         1,
                         mitk::CreateChiliObjectFunction<mitk::LightBoxImageReaderImpl>::New());
  this->RegisterOverride(typeid(mitk::LightBoxResultImageWriter).name(),
                         typeid(mitk::LightBoxResultImageWriterImpl).name(),
                         "MITK Chili Plugin Lightbox Writer",
                         1,
                         mitk::CreateChiliObjectFunction<mitk::LightBoxResultImageWriterImpl>::New());
  ObjectFactoryBase::RegisterFactory(this);
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

mitk::ChiliPluginFactory::Pointer dummyFactory = mitk::ChiliPluginFactory::New();

