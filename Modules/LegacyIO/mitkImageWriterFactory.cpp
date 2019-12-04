/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    typedef CreateImageWriter Self;
    typedef itk::SmartPointer<Self> Pointer;

    /** Methods from itk:LightObject. */
    itkFactorylessNewMacro(Self);
    LightObject::Pointer CreateObject() override
    {
      typename T::Pointer p = T::New();
      p->Register();
      return p.GetPointer();
    }

  protected:
    CreateImageWriter() {}
    ~CreateImageWriter() override {}
  private:
    CreateImageWriter(const Self &); // purposely not implemented
    void operator=(const Self &);    // purposely not implemented
  };

  ImageWriterFactory::ImageWriterFactory()
  {
    this->RegisterOverride(
      "IOWriter", "ImageWriter", "Image Writer", true, mitk::CreateImageWriter<mitk::ImageWriter>::New());
  }

  ImageWriterFactory::~ImageWriterFactory() {}
  const char *ImageWriterFactory::GetITKSourceVersion() const { return ITK_SOURCE_VERSION; }
  const char *ImageWriterFactory::GetDescription() const { return "ImageWriterFactory"; }
} // end namespace mitk
