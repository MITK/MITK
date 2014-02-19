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

#ifndef TubeGraph_WRITERFACTORY_H_HEADER_INCLUDED
#define TubeGraph_WRITERFACTORY_H_HEADER_INCLUDED

#include "MitkTubeGraphExports.h"

#include "itkObjectFactoryBase.h"
#include "mitkBaseData.h"

namespace mitk
{

class MitkTubeGraph_EXPORT TubeGraphWriterFactory : public itk::ObjectFactoryBase
{
public:

  mitkClassMacro( mitk::TubeGraphWriterFactory, itk::ObjectFactoryBase )

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
      TubeGraphWriterFactory::Pointer cnfWriterFactory = TubeGraphWriterFactory::New();
      ObjectFactoryBase::RegisterFactory( cnfWriterFactory );
      IsRegistered = true;
    }
  }

protected:
  TubeGraphWriterFactory();
  ~TubeGraphWriterFactory();

private:
  TubeGraphWriterFactory(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};

} // end namespace mitk

#endif // TubeGraph_WRITERFACTORY_H_HEADER_INCLUDED
