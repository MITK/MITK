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

#include "mitkConnectomicsNetworkWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include "mitkConnectomicsNetworkWriter.h"

namespace mitk
{

template <class T>
class CreateConnectomicsNetworkWriter : public itk::CreateObjectFunctionBase
{
public:

  /** Standard class typedefs. */
  typedef CreateConnectomicsNetworkWriter  Self;
  typedef itk::SmartPointer<Self>    Pointer;

  /** Methods from itk:LightObject. */
  itkFactorylessNewMacro(Self);
  LightObject::Pointer CreateObject() { typename T::Pointer p = T::New();
    p->Register();
    return p.GetPointer();
  }

protected:
  CreateConnectomicsNetworkWriter() {}
  ~CreateConnectomicsNetworkWriter() {}

private:
  CreateConnectomicsNetworkWriter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

ConnectomicsNetworkWriterFactory::ConnectomicsNetworkWriterFactory()
{
  this->RegisterOverride("IOWriter",
                         "ConnectomicsNetworkWriter",
                         "ConnectomicsNetwork Writer",
                         1,
                         mitk::CreateConnectomicsNetworkWriter< mitk::ConnectomicsNetworkWriter >::New());
}

ConnectomicsNetworkWriterFactory::~ConnectomicsNetworkWriterFactory()
{
}

const char* ConnectomicsNetworkWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* ConnectomicsNetworkWriterFactory::GetDescription() const
{
  return "ConnectomicsNetworkWriterFactory";
}

} // end namespace mitk
