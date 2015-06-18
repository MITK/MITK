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

#include "mitkImageWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include "mitkImageWriter.h"

namespace mitk
{

template <class T>
class CreateImageWriter : public itk::CreateObjectFunctionBase
{
public:

  /** Standard class typedefs. */
  typedef CreateImageWriter  Self;
  typedef itk::SmartPointer<Self>    Pointer;

  /** Methods from itk:LightObject. */
  itkFactorylessNewMacro(Self);
  LightObject::Pointer CreateObject() override { typename T::Pointer p = T::New();
    p->Register();
    return p.GetPointer();
  }

protected:
  CreateImageWriter() {}
  ~CreateImageWriter() {}

private:
  CreateImageWriter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

ImageWriterFactory::ImageWriterFactory()
{
  this->RegisterOverride("IOWriter",
                         "ImageWriter",
                         "Image Writer",
                         1,
                         mitk::CreateImageWriter<mitk::ImageWriter>::New());
}

ImageWriterFactory::~ImageWriterFactory()
{
}

const char* ImageWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* ImageWriterFactory::GetDescription() const
{
  return "ImageWriterFactory";
}

} // end namespace mitk
