/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkVtkSurfaceIOFactory_h
#define mitkVtkSurfaceIOFactory_h

#include <MitkLegacyIOExports.h>

#include <itkObjectFactoryBase.h>
#include <mitkBaseData.h>

namespace mitk
{
  /**
   * \brief Create instances of VtkSurfaceReader objects using an object factory.
   *
   * \ingroup MitkLegacyIOModule
   */
  class MITKLEGACYIO_EXPORT VtkSurfaceIOFactory : public itk::ObjectFactoryBase
  {
  public:
    /** Standard class typedefs. */
    typedef VtkSurfaceIOFactory Self;
    typedef itk::ObjectFactoryBase Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** Class methods used to interface with the registered factories. */
    const char *GetITKSourceVersion(void) const override;
    const char *GetDescription(void) const override;

    /** Method for class instantiation. */
    itkFactorylessNewMacro(Self);
    static VtkSurfaceIOFactory *FactoryNew() { return new VtkSurfaceIOFactory; }
    /** Run-time type information (and related methods). */
    itkTypeMacro(VtkSurfaceIOFactory, ObjectFactoryBase);

  protected:
    /** \brief Default constructor. */
    VtkSurfaceIOFactory();
    /** \brief Destructor. */
    ~VtkSurfaceIOFactory() override;

  private:
    VtkSurfaceIOFactory(const Self &); ///< \brief Purposely not implemented.
    void operator=(const Self &);      ///< \brief Purposely not implemented.
  };

} // end namespace mitk

#endif
