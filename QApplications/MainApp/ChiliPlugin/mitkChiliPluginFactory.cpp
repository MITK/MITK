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

#include "mitkChiliPluginFactory.h"

//ITK
#include "itkCreateObjectFunction.h"
#include "itkVersion.h"
//MITK
#include "mitkPACSPlugin.h"
#include "mitkChiliPlugin.h"

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
