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
#ifndef __mitkContourModelIOFactory_h
#define __mitkContourModelIOFactory_h

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include "SegmentationExports.h"
#include "itkObjectFactoryBase.h"
#include "mitkBaseData.h"

namespace mitk
{
//##Documentation
//## @brief Create instances of ContourModelReader objects using an object factory.
//##
//## @ingroup IO
class Segmentation_EXPORT ContourModelIOFactory : public itk::ObjectFactoryBase
{
public:
  /** Standard class typedefs. */
  typedef ContourModelIOFactory   Self;
  typedef itk::ObjectFactoryBase  Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;

  /** Class methods used to interface with the registered factories. */
  virtual const char* GetITKSourceVersion(void) const;
  virtual const char* GetDescription(void) const;

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);
  static ContourModelIOFactory* FactoryNew() { return new ContourModelIOFactory;}
  /** Run-time type information (and related methods). */
  itkTypeMacro(ContourModelIOFactory, ObjectFactoryBase);

  /** Register one factory of this type  */
  static void RegisterOneFactory(void)
  {
    ContourModelIOFactory::Pointer ContourModelIOFactory = ContourModelIOFactory::New();
    ObjectFactoryBase::RegisterFactory(ContourModelIOFactory);
  }

protected:
  ContourModelIOFactory();
  ~ContourModelIOFactory();

private:
  ContourModelIOFactory(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};


} // end namespace mitk

#endif
