/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
  typedef CreatePointSetWriter  Self;
  typedef itk::SmartPointer<Self>    Pointer;

  /** Methods from itk:LightObject. */
  itkFactorylessNewMacro(Self);
  LightObject::Pointer CreateObject() { typename T::Pointer p = T::New();
    p->Register();
    return p.GetPointer();
  }

protected:
  CreatePointSetWriter() {}
  ~CreatePointSetWriter() {}

private:
  CreatePointSetWriter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

PointSetWriterFactory::PointSetWriterFactory()
{
  this->RegisterOverride("IOWriter",
                         "PointSetWriter",
                         "Point-Set Writer",
                         1,
                         mitk::CreatePointSetWriter<mitk::PointSetWriter>::New());
}

PointSetWriterFactory::~PointSetWriterFactory()
{
}

const char* PointSetWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* PointSetWriterFactory::GetDescription() const
{
  return "PointSetWriterFactory";
}

} // end namespace mitk
