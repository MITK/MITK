/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkParRecFileIOFactory_h
#define mitkParRecFileIOFactory_h

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include "itkObjectFactoryBase.h"
#include "mitkBaseData.h"

namespace mitk
{
  //##Documentation
  //## @brief Create instances of ParRecFileReader objects using an object factory.
  //##
  class ParRecFileIOFactory : public itk::ObjectFactoryBase
  {
  public:
    /** Standard class typedefs. */
    typedef ParRecFileIOFactory Self;
    typedef itk::ObjectFactoryBase Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** Class methods used to interface with the registered factories. */
    const char *GetITKSourceVersion(void) const override;
    const char *GetDescription(void) const override;

    /** Method for class instantiation. */
    itkFactorylessNewMacro(Self);
    static ParRecFileIOFactory *FactoryNew() { return new ParRecFileIOFactory; }
    /** Run-time type information (and related methods). */
    itkTypeMacro(ParRecFileIOFactory, ObjectFactoryBase);

    /**
     * Register one factory of this type
     * \deprecatedSince{2013_09}
     */
    DEPRECATED(static void RegisterOneFactory(void))
    {
      ParRecFileIOFactory::Pointer ParRecFileIOFactory = ParRecFileIOFactory::New();
      ObjectFactoryBase::RegisterFactory(ParRecFileIOFactory);
    }

  protected:
    ParRecFileIOFactory();
    ~ParRecFileIOFactory() override;

  private:
    ParRecFileIOFactory(const Self &); // purposely not implemented
    void operator=(const Self &);      // purposely not implemented
  };

} // end namespace mitk

#endif
