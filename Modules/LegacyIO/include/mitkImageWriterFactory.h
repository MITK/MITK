/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageWriterFactory_h
#define mitkImageWriterFactory_h

#include <MitkLegacyIOExports.h>

#include <itkObjectFactoryBase.h>
#include <mitkBaseData.h>

namespace mitk
{
  /**
   * \brief ITK object factory for creating mitk::ImageWriter instances.
   *
   * This factory registers mitk::ImageWriter with the ITK object factory
   * mechanism, allowing automatic creation of ImageWriter objects when
   * requested through itk::ObjectFactoryBase.
   *
   * \ingroup MitkLegacyIOModule
   * \deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
   * \sa mitk::ImageWriter
   */
  class DEPRECATED() MITKLEGACYIO_EXPORT ImageWriterFactory : public itk::ObjectFactoryBase
  {
  public:
    mitkClassMacroItkParent(mitk::ImageWriterFactory, itk::ObjectFactoryBase);

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

    /** Method for class instantiation. */
    itkFactorylessNewMacro(Self);

    /**
     * \brief Register one instance of this factory with the ITK object factory system.
     *
     * Ensures the factory is only registered once. Subsequent calls have no effect.
     *
     * \deprecatedSince{2013_09}
     */
    DEPRECATED(static void RegisterOneFactory(void))
    {
      static bool IsRegistered = false;
      if (!IsRegistered)
      {
        ImageWriterFactory::Pointer imageWriterFactory = ImageWriterFactory::New();
        ObjectFactoryBase::RegisterFactory(imageWriterFactory);
        IsRegistered = true;
      }
    }

  protected:
    ImageWriterFactory();
    ~ImageWriterFactory() override;

  private:
    ImageWriterFactory(const Self &); // purposely not implemented
    void operator=(const Self &);     // purposely not implemented
  };

} // end namespace mitk

#endif
