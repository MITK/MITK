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

#ifndef NRRDTBSSIMAGE_WRITERFACTORY_H_HEADER_INCLUDED
#define NRRDTBSSIMAGE_WRITERFACTORY_H_HEADER_INCLUDED

#include "itkObjectFactoryBase.h"
#include "mitkBaseData.h"

namespace mitk
{

class NrrdTbssImageWriterFactory : public itk::ObjectFactoryBase
{
public:

  mitkClassMacroItkParent( mitk::NrrdTbssImageWriterFactory, itk::ObjectFactoryBase )

  /** Class methods used to interface with the registered factories. */
  const char* GetITKSourceVersion(void) const override;
  const char* GetDescription(void) const override;

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);

protected:
  NrrdTbssImageWriterFactory();
  ~NrrdTbssImageWriterFactory() override;

private:
  NrrdTbssImageWriterFactory(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};

} // end namespace mitk

#endif // NRRDTBSSIMAGE_WRITERFACTORY_H_HEADER_INCLUDED



