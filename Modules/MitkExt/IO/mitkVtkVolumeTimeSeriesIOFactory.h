/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __mitkVtkVolumeTimeSeriesIOFactory_h
#define __mitkVtkVolumeTimeSeriesIOFactory_h

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include "itkObjectFactoryBase.h"
#include "MitkExtExports.h"
#include "mitkBaseData.h"

namespace mitk
{
//##Documentation
//## @brief Create instances of VtkVolumeTimeSeriesReader objects using an object factory.
//##
//## @ingroup IO
class MitkExt_EXPORT VtkVolumeTimeSeriesIOFactory : public itk::ObjectFactoryBase
{
public:  
  /** Standard class typedefs. */
  typedef VtkVolumeTimeSeriesIOFactory   Self;
  typedef itk::ObjectFactoryBase  Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;
  
  /** Class methods used to interface with the registered factories. */
  virtual const char* GetITKSourceVersion(void) const;
  virtual const char* GetDescription(void) const;
    
  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);
  static VtkVolumeTimeSeriesIOFactory* FactoryNew() { return new VtkVolumeTimeSeriesIOFactory;}
  /** Run-time type information (and related methods). */
  itkTypeMacro(VtkVolumeTimeSeriesIOFactory, ObjectFactoryBase);

  /** Register one factory of this type  */
  static void RegisterOneFactory(void)
  {
    VtkVolumeTimeSeriesIOFactory::Pointer VtkVolumeTimeSeriesIOFactory = VtkVolumeTimeSeriesIOFactory::New();
    ObjectFactoryBase::RegisterFactory(VtkVolumeTimeSeriesIOFactory);
  }
  
protected:
  VtkVolumeTimeSeriesIOFactory();
  ~VtkVolumeTimeSeriesIOFactory();

private:
  VtkVolumeTimeSeriesIOFactory(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};
  
  
} // end namespace mitk

#endif
