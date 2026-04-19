/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPointSetWriterFactory_h
#define mitkPointSetWriterFactory_h

#include <MitkLegacyIOExports.h>

#include <itkObjectFactoryBase.h>
#include <mitkBaseData.h>

namespace mitk
{
  /**
   * \brief ITK object factory for creating mitk::PointSetWriter instances.
   *
   * Registers mitk::PointSetWriter with the ITK object factory mechanism,
   * allowing automatic creation of PointSetWriter objects when requested
   * through itk::ObjectFactoryBase.
   *
   * \ingroup MitkLegacyIOModule
   * \sa mitk::PointSetWriter
   */
  class MITKLEGACYIO_EXPORT PointSetWriterFactory : public itk::ObjectFactoryBase
  {
  public:
    mitkClassMacroItkParent(mitk::PointSetWriterFactory, itk::ObjectFactoryBase);

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

  protected:
    PointSetWriterFactory();
    ~PointSetWriterFactory() override;

  private:
    PointSetWriterFactory(const Self &); // purposely not implemented
    void operator=(const Self &);        // purposely not implemented
  };

} // end namespace mitk

#endif
