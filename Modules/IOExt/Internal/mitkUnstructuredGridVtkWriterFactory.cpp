/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUnstructuredGridVtkWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include "mitkUnstructuredGridVtkWriter.h"

#include <vtkUnstructuredGridWriter.h>
#include <vtkXMLPUnstructuredGridWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>

namespace mitk
{
  template <class T>
  class CreateUnstructuredGridWriter : public itk::CreateObjectFunctionBase
  {
  public:
    /** Standard class typedefs. */
    typedef CreateUnstructuredGridWriter Self;
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
    CreateUnstructuredGridWriter() {}
    ~CreateUnstructuredGridWriter() override {}
  private:
    CreateUnstructuredGridWriter(const Self &); // purposely not implemented
    void operator=(const Self &);               // purposely not implemented
  };

  UnstructuredGridVtkWriterFactory::UnstructuredGridVtkWriterFactory()
  {
    this->RegisterOverride(
      "IOWriter",
      "UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>",
      "VTK Legacy Unstructured Grid Writer",
      true,
      mitk::CreateUnstructuredGridWriter<mitk::UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>>::New());

    this->RegisterOverride(
      "IOWriter",
      "UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter>",
      "VTK XML UnstructuredGrid Writer",
      true,
      mitk::CreateUnstructuredGridWriter<mitk::UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter>>::New());

    this->RegisterOverride(
      "IOWriter",
      "UnstructuredGridVtkWriter<vtkXMLPUnstructuredGridWriter>",
      "VTK Parallel XML UnstructuredGrid Writer",
      true,
      mitk::CreateUnstructuredGridWriter<mitk::UnstructuredGridVtkWriter<vtkXMLPUnstructuredGridWriter>>::New());
  }

  UnstructuredGridVtkWriterFactory::~UnstructuredGridVtkWriterFactory() {}
  const char *UnstructuredGridVtkWriterFactory::GetITKSourceVersion() const { return ITK_SOURCE_VERSION; }
  const char *UnstructuredGridVtkWriterFactory::GetDescription() const { return "UnstructuredGridVtkWriterFactory"; }
} // end namespace mitk
