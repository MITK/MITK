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

#include "mitkUnstructuredGridVtkWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include <mitkUnstructuredGridVtkWriter.h>

#include <vtkUnstructuredGridWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkXMLPUnstructuredGridWriter.h>

namespace mitk
{

template <class T>
class CreateUnstructuredGridWriter : public itk::CreateObjectFunctionBase
{
public:

  /** Standard class typedefs. */
  typedef CreateUnstructuredGridWriter  Self;
  typedef itk::SmartPointer<Self>    Pointer;

  /** Methods from itk:LightObject. */
  itkFactorylessNewMacro(Self);
  LightObject::Pointer CreateObject() { typename T::Pointer p = T::New();
    p->Register();
    return p.GetPointer();
  }

protected:
  CreateUnstructuredGridWriter() {}
  ~CreateUnstructuredGridWriter() {}

private:
  CreateUnstructuredGridWriter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

UnstructuredGridVtkWriterFactory::UnstructuredGridVtkWriterFactory()
{
  this->RegisterOverride("IOWriter",
                         "UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>",
                         "VTK Legacy Unstructured Grid Writer",
                         1,
                         mitk::CreateUnstructuredGridWriter< mitk::UnstructuredGridVtkWriter<vtkUnstructuredGridWriter> >::New());

  this->RegisterOverride("IOWriter",
                         "UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter>",
                         "VTK XML UnstructuredGrid Writer",
                         1,
                         mitk::CreateUnstructuredGridWriter< mitk::UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter> >::New());

  this->RegisterOverride("IOWriter",
                         "UnstructuredGridVtkWriter<vtkXMLPUnstructuredGridWriter>",
                         "VTK Parallel XML UnstructuredGrid Writer",
                         1,
                         mitk::CreateUnstructuredGridWriter< mitk::UnstructuredGridVtkWriter<vtkXMLPUnstructuredGridWriter> >::New());
}

UnstructuredGridVtkWriterFactory::~UnstructuredGridVtkWriterFactory()
{
}

const char* UnstructuredGridVtkWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* UnstructuredGridVtkWriterFactory::GetDescription() const
{
  return "UnstructuredGridVtkWriterFactory";
}

} // end namespace mitk
