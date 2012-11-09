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
#ifndef __MITK_CONNECTOMICS_NETWORK_IO_FACTORY_H_HEADER__
#define __MITK_CONNECTOMICS_NETWORK_IO_FACTORY_H_HEADER__

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include "itkObjectFactoryBase.h"
#include "mitkBaseData.h"
#include "MitkDiffusionImagingExports.h"

namespace mitk
{
 /**
   * \brief Create instances of ConnectomicsNetwork objects using an object factory.
   *
   * \ingroup IO
   */
class MitkDiffusionImaging_EXPORT ConnectomicsNetworkIOFactory : public itk::ObjectFactoryBase
{
public:
  /** Standard class typedefs. */
  typedef ConnectomicsNetworkIOFactory   Self;
  typedef itk::ObjectFactoryBase  Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;

  /** Class methods used to interface with the registered factories. */
  virtual const char* GetITKSourceVersion(void) const;
  virtual const char* GetDescription(void) const;

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);
  static ConnectomicsNetworkIOFactory* FactoryNew() { return new ConnectomicsNetworkIOFactory;}
  /** Run-time type information (and related methods). */
  itkTypeMacro(ConnectomicsNetworkIOFactory, ObjectFactoryBase);

  /** Register one factory of this type  */
  static void RegisterOneFactory(void)
  {
    ConnectomicsNetworkIOFactory::Pointer ConnectomicsNetworkIOFactory = ConnectomicsNetworkIOFactory::New();
    ObjectFactoryBase::RegisterFactory(ConnectomicsNetworkIOFactory);
  }

protected:
  ConnectomicsNetworkIOFactory();
  ~ConnectomicsNetworkIOFactory();

private:
  ConnectomicsNetworkIOFactory(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};


} // end namespace mitk

#endif  // __MITK_CONNECTOMICS_NETWORK_IO_FACTORY_H_HEADER__
