/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkVtiFileIOFactory_h
#define mitkVtiFileIOFactory_h

#include <MitkLegacyIOExports.h>

#include <itkObjectFactoryBase.h>
#include <mitkBaseData.h>

namespace mitk
{
  /**
   * \brief ITK object factory for creating VtiFileReader instances.
   *
   * Registers mitk::VtiFileReader with the ITK object factory mechanism,
   * enabling automatic creation of readers for VTK XML ImageData (.vti) files.
   *
   * \ingroup MitkLegacyIOModule
   * \sa mitk::VtiFileReader
   */
  class MITKLEGACYIO_EXPORT VtiFileIOFactory : public itk::ObjectFactoryBase
  {
  public:
    /** \brief Standard ITK self type. */
    typedef VtiFileIOFactory Self;
    /** \brief Standard ITK superclass type. */
    typedef itk::ObjectFactoryBase Superclass;
    /** \brief Smart pointer type. */
    typedef itk::SmartPointer<Self> Pointer;
    /** \brief Const smart pointer type. */
    typedef itk::SmartPointer<const Self> ConstPointer;

    /**
     * \brief Return the ITK source version string.
     * \return A string identifying the ITK source version.
     */
    const char *GetITKSourceVersion(void) const override;

    /**
     * \brief Return a description of this factory.
     * \return A human-readable description string.
     */
    const char *GetDescription(void) const override;

    /** \brief Method for class instantiation. */
    itkFactorylessNewMacro(Self);

    /**
     * \brief Create a new instance of this factory.
     * \return A raw pointer to a new VtiFileIOFactory instance.
     */
    static VtiFileIOFactory *FactoryNew() { return new VtiFileIOFactory; }

    /** \brief Run-time type information (and related methods). */
    itkTypeMacro(VtiFileIOFactory, ObjectFactoryBase);

  protected:
    VtiFileIOFactory();
    ~VtiFileIOFactory() override;

  private:
    VtiFileIOFactory(const Self &); // purposely not implemented
    void operator=(const Self &);   // purposely not implemented
  };

} // end namespace mitk

#endif
