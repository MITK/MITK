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
#include "mitkBaseData.h"

namespace mitk
{

class PlanarFigureWriterFactory : public itk::ObjectFactoryBase
{
public:

  mitkClassMacroItkParent( mitk::PlanarFigureWriterFactory, itk::ObjectFactoryBase )

  /** Class methods used to interface with the registered factories. */
  virtual const char* GetITKSourceVersion(void) const override;
  virtual const char* GetDescription(void) const override;

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self)

  /**
   * Register one factory of this type
   * \deprecatedSince{2013_09}
   */
  DEPRECATED(static void RegisterOneFactory(void));

  /**
   * UnRegister one factory of this type
   * \deprecatedSince{2013_09}
   */
  DEPRECATED(static void UnRegisterOneFactory(void));

protected:
  PlanarFigureWriterFactory();
  ~PlanarFigureWriterFactory();

private:
  PlanarFigureWriterFactory(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  static itk::ObjectFactoryBase::Pointer GetInstance();
};

} // end namespace mitk

#endif // PLANARFIGURE_WRITERFACTORY_H_HEADER_INCLUDED



