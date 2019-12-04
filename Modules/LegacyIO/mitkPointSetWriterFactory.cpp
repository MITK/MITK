/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPointSetWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include "mitkPointSetWriter.h"

namespace mitk
{
  template <class T>
  class CreatePointSetWriter : public itk::CreateObjectFunctionBase
  {
  public:
    /** Standard class typedefs. */
    typedef CreatePointSetWriter Self;
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
    CreatePointSetWriter() {}
    ~CreatePointSetWriter() override {}
  private:
    CreatePointSetWriter(const Self &); // purposely not implemented
    void operator=(const Self &);       // purposely not implemented
  };

  PointSetWriterFactory::PointSetWriterFactory()
  {
    this->RegisterOverride(
      "IOWriter", "PointSetWriter", "Point-Set Writer", true, mitk::CreatePointSetWriter<mitk::PointSetWriter>::New());
  }

  PointSetWriterFactory::~PointSetWriterFactory() {}
  const char *PointSetWriterFactory::GetITKSourceVersion() const { return ITK_SOURCE_VERSION; }
  const char *PointSetWriterFactory::GetDescription() const { return "PointSetWriterFactory"; }
} // end namespace mitk
