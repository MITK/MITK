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
#ifndef __MITK_NRRD_TBSS_VOULMES_IO_FACTORY_H_HEADER__
#define __MITK_NRRD_TBSS_VOULMES_IO_FACTORY_H_HEADER__

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include "itkObjectFactoryBase.h"
#include "mitkBaseData.h"

namespace mitk
{
//##Documentation
//## @brief Create instances of NrrdDiffusionImageReader objects using an object factory.
//##
class NrrdTbssImageIOFactory : public itk::ObjectFactoryBase
{
public:
  /** Standard class typedefs. */
  typedef NrrdTbssImageIOFactory   Self;
  typedef itk::ObjectFactoryBase  Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;

  /** Class methods used to interface with the registered factories. */
  const char* GetITKSourceVersion(void) const override;
  const char* GetDescription(void) const override;

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);
  static NrrdTbssImageIOFactory* FactoryNew() { return new NrrdTbssImageIOFactory;}
  /** Run-time type information (and related methods). */
  itkTypeMacro(NrrdTbssImageIOFactory, ObjectFactoryBase);

protected:
  NrrdTbssImageIOFactory();
  ~NrrdTbssImageIOFactory() override;

private:
  NrrdTbssImageIOFactory(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};


} // end namespace mitk

#endif  // __MITK_NRRD_TBSS_VOULMES_IO_FACTORY_H_HEADER__
