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

#ifndef PLANARFIGURE_WRITERFACTORY_H_HEADER_INCLUDED
#define PLANARFIGURE_WRITERFACTORY_H_HEADER_INCLUDED

#include "itkObjectFactoryBase.h"
#include "PlanarFigureExports.h"
#include "mitkBaseData.h"

namespace mitk
{

class PlanarFigure_EXPORT PlanarFigureWriterFactory : public itk::ObjectFactoryBase
{
public:

  mitkClassMacro( mitk::PlanarFigureWriterFactory, itk::ObjectFactoryBase )

  /** Class methods used to interface with the registered factories. */
  virtual const char* GetITKSourceVersion(void) const;
  virtual const char* GetDescription(void) const;

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);

  /** Register one factory of this type  */
  static void RegisterOneFactory(void)
  {
    static bool IsRegistered = false;
    if ( !IsRegistered )
    {
      PlanarFigureWriterFactory::Pointer pfwf = PlanarFigureWriterFactory::New();
      ObjectFactoryBase::RegisterFactory( pfwf );
      IsRegistered = true;
    }
  }

protected:
  PlanarFigureWriterFactory();
  ~PlanarFigureWriterFactory();

private:
  PlanarFigureWriterFactory(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};

} // end namespace mitk

#endif // PLANARFIGURE_WRITERFACTORY_H_HEADER_INCLUDED



