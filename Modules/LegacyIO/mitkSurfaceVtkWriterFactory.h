/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef SURFACEWRITERFACTORY_H_HEADER_INCLUDED
#define SURFACEWRITERFACTORY_H_HEADER_INCLUDED

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

  mitkClassMacroItkParent( mitk::SurfaceVtkWriterFactory, itk::ObjectFactoryBase )

  /** Class methods used to interface with the registered factories. */
  virtual const char* GetITKSourceVersion(void) const override;
  virtual const char* GetDescription(void) const override;

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);

  /**
   * Register one factory of this type
   * \deprecatedSince{2013_09}
   */
  DEPRECATED(static void RegisterOneFactory(void))
  {
    static bool IsRegistered = false;
    if ( !IsRegistered )
    {
      SurfaceVtkWriterFactory::Pointer surfaceVtkWriterFactory = SurfaceVtkWriterFactory::New();
      ObjectFactoryBase::RegisterFactory( surfaceVtkWriterFactory );
      IsRegistered = true;
    }
  }

protected:
  SurfaceVtkWriterFactory();
  ~SurfaceVtkWriterFactory();

private:
  SurfaceVtkWriterFactory(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};

} // end namespace mitk

#endif
