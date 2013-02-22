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

#include "mitkFiberBundleXWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include "mitkFiberBundleXWriter.h"

namespace mitk
{

template <class T>
class CreateFiberBundleXWriter : public itk::CreateObjectFunctionBase
{
public:

  /** Standard class typedefs. */
  typedef CreateFiberBundleXWriter  Self;
  typedef itk::SmartPointer<Self>    Pointer;

  /** Methods from itk:LightObject. */
  itkFactorylessNewMacro(Self);
  LightObject::Pointer CreateObject() { typename T::Pointer p = T::New();
    p->Register();
    return p.GetPointer();
  }

protected:
  CreateFiberBundleXWriter() {}
  ~CreateFiberBundleXWriter() {}

private:
  CreateFiberBundleXWriter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

FiberBundleXWriterFactory::FiberBundleXWriterFactory()
{
  this->RegisterOverride("IOWriter",
                         "FiberBundleXWriter",
                         "FiberBundleX Writer",
                         1,
                         mitk::CreateFiberBundleXWriter< mitk::FiberBundleXWriter >::New());
}

FiberBundleXWriterFactory::~FiberBundleXWriterFactory()
{
}

const char* FiberBundleXWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* FiberBundleXWriterFactory::GetDescription() const
{
  return "FiberBundleXWriterFactory";
}

} // end namespace mitk
