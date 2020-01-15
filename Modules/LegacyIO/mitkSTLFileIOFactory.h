/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __mitkSTLFileIOFactory_h
#define __mitkSTLFileIOFactory_h

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include <MitkLegacyIOExports.h>

#include "itkObjectFactoryBase.h"
#include "mitkBaseData.h"

namespace mitk
{
  //##Documentation
  //## @brief Create instances of STLFileReader objects using an object factory.
  //##
  //## @ingroup MitkLegacyIOModule
  //## @deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
  class DEPRECATED() MITKLEGACYIO_EXPORT STLFileIOFactory : public itk::ObjectFactoryBase
  {
  public:
    /** Standard class typedefs. */
    typedef STLFileIOFactory Self;
    typedef itk::ObjectFactoryBase Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** Class methods used to interface with the registered factories. */
    const char *GetITKSourceVersion(void) const override;
    const char *GetDescription(void) const override;

    /** Method for class instantiation. */
    itkFactorylessNewMacro(Self);
    static STLFileIOFactory *FactoryNew() { return new STLFileIOFactory; }
    /** Run-time type information (and related methods). */
    itkTypeMacro(STLFileIOFactory, ObjectFactoryBase);

    /**
     * Register one factory of this type
     * \deprecatedSince{2013_09}
     */
    DEPRECATED(static void RegisterOneFactory(void))
    {
      STLFileIOFactory::Pointer STLFileIOFactory = STLFileIOFactory::New();
      ObjectFactoryBase::RegisterFactory(STLFileIOFactory);
    }

  protected:
    STLFileIOFactory();
    ~STLFileIOFactory() override;

  private:
    STLFileIOFactory(const Self &); // purposely not implemented
    void operator=(const Self &);   // purposely not implemented
  };

} // end namespace mitk

#endif
