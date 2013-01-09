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

#include "mitkContourModelWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include <mitkContourModelWriter.h>

namespace mitk
{

template <class T>
class CreateContourModelWriter : public itk::CreateObjectFunctionBase
{
public:

  /** Standard class typedefs. */
  typedef CreateContourModelWriter  Self;
  typedef itk::SmartPointer<Self>    Pointer;

  /** Methods from itk:LightObject. */
  itkFactorylessNewMacro(Self);
  LightObject::Pointer CreateObject() { typename T::Pointer p = T::New();
    p->Register();
    return p.GetPointer();
  }

protected:
  CreateContourModelWriter() {}
  ~CreateContourModelWriter() {}

private:
  CreateContourModelWriter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

ContourModelWriterFactory::ContourModelWriterFactory()
{
  this->RegisterOverride("IOWriter",
                         "ContourModelWriter",
                         "ContourModel xml Writer",
                         1,
                         mitk::CreateContourModelWriter<mitk::ContourModelWriter>::New());
}

ContourModelWriterFactory::~ContourModelWriterFactory()
{
}

const char* ContourModelWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* ContourModelWriterFactory::GetDescription() const
{
  return "ContourModelWriterFactory";
}

} // end namespace mitk
