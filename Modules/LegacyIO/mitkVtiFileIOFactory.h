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
#ifndef __mitkVtiFileIOFactory_h
#define __mitkVtiFileIOFactory_h

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include <MitkLegacyIOExports.h>

#include "itkObjectFactoryBase.h"
#include "mitkBaseData.h"

namespace mitk
{
//##Documentation
//## @brief Create instances of VtiFileReader objects using an object factory.
//##
//## @ingroup MitkLegacyIOModule
//## @deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
class DEPRECATED() MITKLEGACYIO_EXPORT VtiFileIOFactory : public itk::ObjectFactoryBase
{
public:
  /** Standard class typedefs. */
  typedef VtiFileIOFactory   Self;
  typedef itk::ObjectFactoryBase  Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;

  /** Class methods used to interface with the registered factories. */
  virtual const char* GetITKSourceVersion(void) const;
  virtual const char* GetDescription(void) const;

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);
  static VtiFileIOFactory* FactoryNew() { return new VtiFileIOFactory;}
  /** Run-time type information (and related methods). */
  itkTypeMacro(VtiFileIOFactory, ObjectFactoryBase);

  /**
   * Register one factory of this type
   * \deprecatedSince{2013_09}
   */
  DEPRECATED(static void RegisterOneFactory(void))
  {
    VtiFileIOFactory::Pointer VtiFileIOFactory = VtiFileIOFactory::New();
    ObjectFactoryBase::RegisterFactory(VtiFileIOFactory);
  }

protected:
  VtiFileIOFactory();
  ~VtiFileIOFactory();

private:
  VtiFileIOFactory(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};


} // end namespace mitk

#endif
