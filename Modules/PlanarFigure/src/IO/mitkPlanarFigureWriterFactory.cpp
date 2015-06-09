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

#include "mitkPlanarFigureWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include <mitkPlanarFigureWriter.h>

namespace mitk
{

template <class T>
class CreatePlanarFigureWriter : public itk::CreateObjectFunctionBase
{
public:

  /** Standard class typedefs. */
  typedef CreatePlanarFigureWriter  Self;
  typedef itk::SmartPointer<Self>    Pointer;

  /** Methods from itk:LightObject. */
  itkFactorylessNewMacro(Self)
  LightObject::Pointer CreateObject() override
  {
    typename T::Pointer p = T::New();
    p->Register();
    return p.GetPointer();
  }

protected:
  CreatePlanarFigureWriter() {}
  ~CreatePlanarFigureWriter() {}

private:
  CreatePlanarFigureWriter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

PlanarFigureWriterFactory::PlanarFigureWriterFactory()
{
  this->RegisterOverride("IOWriter",
                         "PlanarFigureWriter",
                         "PlanarFigure xml Writer",
                         1,
                         mitk::CreatePlanarFigureWriter< mitk::PlanarFigureWriter >::New());
}

PlanarFigureWriterFactory::~PlanarFigureWriterFactory()
{
}

itk::ObjectFactoryBase::Pointer PlanarFigureWriterFactory::GetInstance()
{
  static itk::ObjectFactoryBase::Pointer factory(mitk::PlanarFigureWriterFactory::New().GetPointer());
  return factory;
}

void PlanarFigureWriterFactory::RegisterOneFactory(void)
{
  if ( GetInstance()->GetReferenceCount() == 1 )
  {
    ObjectFactoryBase::RegisterFactory( GetInstance().GetPointer() );
  }
}

void PlanarFigureWriterFactory::UnRegisterOneFactory(void)
{
  ObjectFactoryBase::UnRegisterFactory( GetInstance().GetPointer() );
}

const char* PlanarFigureWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* PlanarFigureWriterFactory::GetDescription() const
{
  return "PlanarFigureWriterFactory";
}

} // end namespace mitk
