/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 11215 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkUnstructuredGridVtkWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include <mitkUnstructuredGridVtkWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>

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
                         "UnstructuredGridVtkWriter",
                         "UnstructuredGrid Vtk Writer",
                         1,
                         mitk::CreateUnstructuredGridWriter< mitk::UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter> >::New());
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
