/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef POINTSETWRITERFACTORY_H_HEADER_INCLUDED
#define POINTSETWRITERFACTORY_H_HEADER_INCLUDED

#include <MitkLegacyIOExports.h>

#include "itkObjectFactoryBase.h"
#include "mitkBaseData.h"

namespace mitk
{
  /**
   * @deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
   */
  class DEPRECATED() MITKLEGACYIO_EXPORT PointSetWriterFactory : public itk::ObjectFactoryBase
  {
  public:
    mitkClassMacroItkParent(mitk::PointSetWriterFactory, itk::ObjectFactoryBase);

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
        PointSetWriterFactory::Pointer pointSetWriterFactory = PointSetWriterFactory::New();
        ObjectFactoryBase::RegisterFactory(pointSetWriterFactory);
        IsRegistered = true;
      }
    }

  protected:
    PointSetWriterFactory();
    ~PointSetWriterFactory() override;

  private:
    PointSetWriterFactory(const Self &); // purposely not implemented
    void operator=(const Self &);        // purposely not implemented
  };

} // end namespace mitk

#endif
