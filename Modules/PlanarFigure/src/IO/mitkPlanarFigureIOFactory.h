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
#ifndef __mitkPlanarFigureIOFactory_h
#define __mitkPlanarFigureIOFactory_h

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include "itkObjectFactoryBase.h"
#include "mitkBaseData.h"

namespace mitk
{
//##Documentation
//## @brief Create instances of PlanarFigureReader objects using an object factory.
//##
//## @ingroup MitkPlanarFigureModule
class PlanarFigureIOFactory : public itk::ObjectFactoryBase
{
public:
  /** Standard class typedefs. */
  typedef PlanarFigureIOFactory   Self;
  typedef itk::ObjectFactoryBase  Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;

  /** Class methods used to interface with the registered factories. */
  virtual const char* GetITKSourceVersion(void) const override;
  virtual const char* GetDescription(void) const override;

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);
  static PlanarFigureIOFactory* FactoryNew() { return new PlanarFigureIOFactory;}
  /** Run-time type information (and related methods). */
  itkTypeMacro(PlanarFigureIOFactory, ObjectFactoryBase);

  /**
   * Register one factory of this type
   * \deprecatedSince{2013_09}
   */
  DEPRECATED(static void RegisterOneFactory(void))
  {
    PlanarFigureIOFactory::Pointer PlanarFigureIOFactory = PlanarFigureIOFactory::New();
    ObjectFactoryBase::RegisterFactory(PlanarFigureIOFactory);
  }

protected:
  PlanarFigureIOFactory();
  ~PlanarFigureIOFactory();

private:
  PlanarFigureIOFactory(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};


} // end namespace mitk

#endif
