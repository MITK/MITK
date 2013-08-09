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

#include "mitkNrrdLabelSetImageWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include <mitkNrrdLabelSetImageWriter.h>

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

NrrdLabelSetImageWriterFactory::NrrdLabelSetImageWriterFactory()
{
  typedef unsigned char LabelSetPixelType;
  this->RegisterOverride("IOWriter",
                         "NrrdLabelSetImageWriter",
                         "NrrdLabelSetImage Writer",
                         1,
                         itk::CreateObjectFunction< mitk::NrrdLabelSetImageWriter<LabelSetPixelType> >::New());
}

NrrdLabelSetImageWriterFactory::~NrrdLabelSetImageWriterFactory()
{
}

const char* NrrdLabelSetImageWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* NrrdLabelSetImageWriterFactory::GetDescription() const
{
  return "NrrdLabelSetImageWriterFactory";
}

} // end namespace mitk
