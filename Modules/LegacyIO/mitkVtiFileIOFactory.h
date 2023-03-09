/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkVtiFileIOFactory_h
#define mitkVtiFileIOFactory_h

#ifdef _MSC_VER
#pragma warning(disable : 4786)
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
    typedef VtiFileIOFactory Self;
    typedef itk::ObjectFactoryBase Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** Class methods used to interface with the registered factories. */
    const char *GetITKSourceVersion(void) const override;
    const char *GetDescription(void) const override;

    /** Method for class instantiation. */
    itkFactorylessNewMacro(Self);
    static VtiFileIOFactory *FactoryNew() { return new VtiFileIOFactory; }
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
    ~VtiFileIOFactory() override;

  private:
    VtiFileIOFactory(const Self &); // purposely not implemented
    void operator=(const Self &);   // purposely not implemented
  };

} // end namespace mitk

#endif
