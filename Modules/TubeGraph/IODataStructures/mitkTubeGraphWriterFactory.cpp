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

#include "mitkTubeGraphWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include "mitkTubeGraphWriter.h"

namespace mitk
{

  template <class T>
  class CreateTubeGraphWriter : public itk::CreateObjectFunctionBase
  {
  public:

    /** Standard class typedefs. */
    typedef CreateTubeGraphWriter  Self;
    typedef itk::SmartPointer<Self>    Pointer;

    /** Methods from itk:LightObject. */
    itkFactorylessNewMacro(Self);
    LightObject::Pointer CreateObject() { typename T::Pointer p = T::New();
    p->Register();
    return p.GetPointer();
    }

  protected:
    CreateTubeGraphWriter() {}
    ~CreateTubeGraphWriter() {}

  private:
    CreateTubeGraphWriter(const Self&); //purposely not implemented
    void operator=(const Self&); //purposely not implemented
  };

  TubeGraphWriterFactory::TubeGraphWriterFactory()
  {
    this->RegisterOverride("IOWriter",
      "TubeGraphWriter",
      "TubeGraph Writer",
      1,
      mitk::CreateTubeGraphWriter< mitk::TubeGraphWriter >::New());
  }

  TubeGraphWriterFactory::~TubeGraphWriterFactory()
  {
  }

  const char* TubeGraphWriterFactory::GetITKSourceVersion() const
  {
    return ITK_SOURCE_VERSION;
  }

  const char* TubeGraphWriterFactory::GetDescription() const
  {
    return "TubeGraphWriterFactory";
  }
} // end namespace mitk
