/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __mitkVtkVolumeTimeSeriesIOFactory_h
#define __mitkVtkVolumeTimeSeriesIOFactory_h

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include "itkObjectFactoryBase.h"
#include "mitkBaseData.h"

namespace mitk
{
  //##Documentation
  //## @brief Create instances of VtkVolumeTimeSeriesReader objects using an object factory.
  class VtkVolumeTimeSeriesIOFactory : public itk::ObjectFactoryBase
  {
  public:
    /** Standard class typedefs. */
    typedef VtkVolumeTimeSeriesIOFactory Self;
    typedef itk::ObjectFactoryBase Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** Class methods used to interface with the registered factories. */
    const char *GetITKSourceVersion(void) const override;
    const char *GetDescription(void) const override;

    /** Method for class instantiation. */
    itkFactorylessNewMacro(Self);
    static VtkVolumeTimeSeriesIOFactory *FactoryNew() { return new VtkVolumeTimeSeriesIOFactory; }
    /** Run-time type information (and related methods). */
    itkTypeMacro(VtkVolumeTimeSeriesIOFactory, ObjectFactoryBase);

    /**
     * Register one factory of this type
     * \deprecatedSince{2013_09}
     */
    DEPRECATED(static void RegisterOneFactory(void))
    {
      VtkVolumeTimeSeriesIOFactory::Pointer VtkVolumeTimeSeriesIOFactory = VtkVolumeTimeSeriesIOFactory::New();
      ObjectFactoryBase::RegisterFactory(VtkVolumeTimeSeriesIOFactory);
    }

  protected:
    VtkVolumeTimeSeriesIOFactory();
    ~VtkVolumeTimeSeriesIOFactory() override;

  private:
    VtkVolumeTimeSeriesIOFactory(const Self &); // purposely not implemented
    void operator=(const Self &);               // purposely not implemented
  };

} // end namespace mitk

#endif
