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

#include "mitkNrrdTbssImageIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkNrrdTbssImageReader.h"

#include "itkVersion.h"


namespace mitk
{

NrrdTbssImageIOFactory::NrrdTbssImageIOFactory()
{
  typedef NrrdTbssImageReader NrrdTbssVolReaderType;
  this->RegisterOverride("mitkIOAdapter",
                         "mitkNrrdTbssImageReader",
                         "mitk Tbss Image IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<NrrdTbssVolReaderType> >::New());
}

NrrdTbssImageIOFactory::~NrrdTbssImageIOFactory()
{
}

const char* NrrdTbssImageIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* NrrdTbssImageIOFactory::GetDescription() const
{
  return "NrrdTbssImage IO Factory, allows the loading of NRRD Tbss data";
}

} // end namespace mitk
