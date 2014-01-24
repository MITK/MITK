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

#include "mitkNrrdQBallImageWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include <mitkNrrdQBallImageWriter.h>

namespace mitk
{

//template <class T>
//class CreateNrrdQBallImageWriter : public itk::CreateObjectFunctionBase
//{
//public:

//  /** Standard class typedefs. */
//  typedef CreateNrrdQBallImageWriter  Self;
//  typedef itk::SmartPointer<Self>    Pointer;

//  /** Methods from itk:LightObject. */
//  itkFactorylessNewMacro(Self);
//  LightObject::Pointer CreateObject() { typename T::Pointer p = T::New();
//    p->Register();
//    return p.GetPointer();
//  }

//protected:
//  CreateNrrdQBallImageWriter() {}
//  ~CreateNrrdQBallImageWriter() {}

//private:
//  CreateNrrdQBallImageWriter(const Self&); //purposely not implemented
//  void operator=(const Self&); //purposely not implemented
//};

NrrdQBallImageWriterFactory::NrrdQBallImageWriterFactory()
{
  typedef short DiffusionPixelType;
  this->RegisterOverride("IOWriter",
                         "NrrdQBallImageWriter",
                         "NrrdQBallImage Writer",
                         1,
                         itk::CreateObjectFunction< mitk::NrrdQBallImageWriter >::New());
}

NrrdQBallImageWriterFactory::~NrrdQBallImageWriterFactory()
{
}

const char* NrrdQBallImageWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* NrrdQBallImageWriterFactory::GetDescription() const
{
  return "NrrdQBallImageWriterFactory";
}

} // end namespace mitk
