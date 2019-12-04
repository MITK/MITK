/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSurfaceVtkWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include <mitkSurfaceVtkWriter.h>
#include <vtkXMLPolyDataWriter.h>

namespace mitk
{
  template <class T>
  class CreateSurfaceWriter : public itk::CreateObjectFunctionBase
  {
  public:
    /** Standard class typedefs. */
    typedef CreateSurfaceWriter Self;
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
    CreateSurfaceWriter() {}
    ~CreateSurfaceWriter() override {}
  private:
    CreateSurfaceWriter(const Self &); // purposely not implemented
    void operator=(const Self &);      // purposely not implemented
  };

  SurfaceVtkWriterFactory::SurfaceVtkWriterFactory()
  {
    this->RegisterOverride("IOWriter",
                           "SurfaceVtkWriter",
                           "Surface Vtk Writer",
                           true,
                           mitk::CreateSurfaceWriter<mitk::SurfaceVtkWriter<vtkXMLPolyDataWriter>>::New());
  }

  SurfaceVtkWriterFactory::~SurfaceVtkWriterFactory() {}
  const char *SurfaceVtkWriterFactory::GetITKSourceVersion() const { return ITK_SOURCE_VERSION; }
  const char *SurfaceVtkWriterFactory::GetDescription() const { return "SurfaceVtkWriterFactory"; }
} // end namespace mitk
