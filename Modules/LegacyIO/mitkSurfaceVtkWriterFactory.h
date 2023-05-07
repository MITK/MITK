/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceVtkWriterFactory_h
#define mitkSurfaceVtkWriterFactory_h

#include <MitkLegacyIOExports.h>

#include "itkObjectFactoryBase.h"
#include "mitkBaseData.h"

namespace mitk
{
  /**
   * @deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
   */
  class DEPRECATED() MITKLEGACYIO_EXPORT SurfaceVtkWriterFactory : public itk::ObjectFactoryBase
  {
  public:
    mitkClassMacroItkParent(mitk::SurfaceVtkWriterFactory, itk::ObjectFactoryBase);

      /** Class methods used to interface with the registered factories. */
      const char *GetITKSourceVersion(void) const override;
    const char *GetDescription(void) const override;

    /** Method for class instantiation. */
    itkFactorylessNewMacro(Self);

    /**
     * Register one factory of this type
     * \deprecatedSince{2013_09}
     */
    DEPRECATED(static void RegisterOneFactory(void))
    {
      static bool IsRegistered = false;
      if (!IsRegistered)
      {
        SurfaceVtkWriterFactory::Pointer surfaceVtkWriterFactory = SurfaceVtkWriterFactory::New();
        ObjectFactoryBase::RegisterFactory(surfaceVtkWriterFactory);
        IsRegistered = true;
      }
    }

  protected:
    SurfaceVtkWriterFactory();
    ~SurfaceVtkWriterFactory() override;

  private:
    SurfaceVtkWriterFactory(const Self &); // purposely not implemented
    void operator=(const Self &);          // purposely not implemented
  };

} // end namespace mitk

#endif
