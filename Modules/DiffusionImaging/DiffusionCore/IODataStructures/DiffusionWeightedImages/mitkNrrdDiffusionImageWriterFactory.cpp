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

#include "mitkNrrdDiffusionImageWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include <mitkNrrdDiffusionImageWriter.h>

namespace mitk
{

//template <class T>
//class CreateNrrdDiffusionImageWriter : public itk::CreateObjectFunctionBase
//{
//public:

//  /** Standard class typedefs. */
//  typedef CreateNrrdDiffusionImageWriter  Self;
//  typedef itk::SmartPointer<Self>    Pointer;

//  /** Methods from itk:LightObject. */
//  itkFactorylessNewMacro(Self);
//  LightObject::Pointer CreateObject() { typename T::Pointer p = T::New();
//    p->Register();
//    return p.GetPointer();
//  }

//protected:
//  CreateNrrdDiffusionImageWriter() {}
//  ~CreateNrrdDiffusionImageWriter() {}

//private:
//  CreateNrrdDiffusionImageWriter(const Self&); //purposely not implemented
//  void operator=(const Self&); //purposely not implemented
//};

NrrdDiffusionImageWriterFactory::NrrdDiffusionImageWriterFactory()
{
  typedef short DiffusionPixelType;
  this->RegisterOverride("IOWriter",
                         "NrrdDiffusionImageWriter",
                         "NrrdDiffusionImage Writer",
                         1,
                         itk::CreateObjectFunction< mitk::NrrdDiffusionImageWriter<DiffusionPixelType> >::New());
}

NrrdDiffusionImageWriterFactory::~NrrdDiffusionImageWriterFactory()
{
}

const char* NrrdDiffusionImageWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* NrrdDiffusionImageWriterFactory::GetDescription() const
{
  return "NrrdDiffusionImageWriterFactory";
}

} // end namespace mitk
