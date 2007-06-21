/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2006-11-14 14:22:39 +0100 (Di, 14 Nov 2006) $
Version:   $Revision: 8455 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
  typedef CreateSurfaceWriter  Self;
  typedef itk::SmartPointer<Self>    Pointer;

  /** Methods from itk:LightObject. */
  itkFactorylessNewMacro(Self);
  LightObject::Pointer CreateObject() { typename T::Pointer p = T::New();
    p->Register();
    return p.GetPointer();
  }

protected:
  CreateSurfaceWriter() {}
  ~CreateSurfaceWriter() {}

private:
  CreateSurfaceWriter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

SurfaceVtkWriterFactory::SurfaceVtkWriterFactory()
{
  this->RegisterOverride("IOWriter",
                         "SurfaceVtkWriter",
                         "Surface Vtk Writer",
                         1,
                         mitk::CreateSurfaceWriter< mitk::SurfaceVtkWriter<vtkXMLPolyDataWriter> >::New());
}

SurfaceVtkWriterFactory::~SurfaceVtkWriterFactory()
{
}

const char* SurfaceVtkWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* SurfaceVtkWriterFactory::GetDescription() const
{
  return "SurfaceVtkWriterFactory";
}

} // end namespace mitk
