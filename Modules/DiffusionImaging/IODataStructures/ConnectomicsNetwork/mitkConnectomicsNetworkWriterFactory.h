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

#ifndef ConnectomicsNetwork_WRITERFACTORY_H_HEADER_INCLUDED
#define ConnectomicsNetwork_WRITERFACTORY_H_HEADER_INCLUDED

#include "itkObjectFactoryBase.h"
#include "mitkBaseData.h"
#include "MitkDiffusionImagingExports.h"

namespace mitk
{

class MitkDiffusionImaging_EXPORT ConnectomicsNetworkWriterFactory : public itk::ObjectFactoryBase
{
public:

  mitkClassMacro( mitk::ConnectomicsNetworkWriterFactory, itk::ObjectFactoryBase )

  /** Class methods used to interface with the registered factories. */
  virtual const char* GetITKSourceVersion(void) const;
  virtual const char* GetDescription(void) const;

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);

  /** Register one factory of this type  */
  static void RegisterOneFactory(void)
  {
    static bool IsRegistered = false;
    if ( !IsRegistered )
    {
      ConnectomicsNetworkWriterFactory::Pointer cnfWriterFactory = ConnectomicsNetworkWriterFactory::New();
      ObjectFactoryBase::RegisterFactory( cnfWriterFactory );
      IsRegistered = true;
    }
  }

protected:
  ConnectomicsNetworkWriterFactory();
  ~ConnectomicsNetworkWriterFactory();

private:
  ConnectomicsNetworkWriterFactory(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};

} // end namespace mitk

#endif // ConnectomicsNetwork_WRITERFACTORY_H_HEADER_INCLUDED



