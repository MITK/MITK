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

#include "mitkNrrdTensorImageWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include <mitkNrrdTensorImageWriter.h>

namespace mitk
{

//template <class T>
//class CreateNrrdTensorImageWriter : public itk::CreateObjectFunctionBase
//{
//public:

//  /** Standard class typedefs. */
//  typedef CreateNrrdTensorImageWriter  Self;
//  typedef itk::SmartPointer<Self>    Pointer;

//  /** Methods from itk:LightObject. */
//  itkFactorylessNewMacro(Self);
//  LightObject::Pointer CreateObject() { typename T::Pointer p = T::New();
//    p->Register();
//    return p.GetPointer();
//  }

//protected:
//  CreateNrrdTensorImageWriter() {}
//  ~CreateNrrdTensorImageWriter() {}

//private:
//  CreateNrrdTensorImageWriter(const Self&); //purposely not implemented
//  void operator=(const Self&); //purposely not implemented
//};

NrrdTensorImageWriterFactory::NrrdTensorImageWriterFactory()
{
  typedef short DiffusionPixelType;
  this->RegisterOverride("IOWriter",
                         "NrrdTensorImageWriter",
                         "NrrdTensorImage Writer",
                         1,
                         itk::CreateObjectFunction< mitk::NrrdTensorImageWriter >::New());
}

NrrdTensorImageWriterFactory::~NrrdTensorImageWriterFactory()
{
}

const char* NrrdTensorImageWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* NrrdTensorImageWriterFactory::GetDescription() const
{
  return "NrrdTensorImageWriterFactory";
}

} // end namespace mitk
