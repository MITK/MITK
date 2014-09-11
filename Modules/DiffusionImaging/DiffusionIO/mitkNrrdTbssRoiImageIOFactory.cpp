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

#include "mitkNrrdTbssRoiImageIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkNrrdTbssRoiImageReader.h"

#include "itkVersion.h"


namespace mitk
{

NrrdTbssRoiImageIOFactory::NrrdTbssRoiImageIOFactory()
{
  typedef NrrdTbssRoiImageReader NrrdTbssRoiVolReaderType;
  this->RegisterOverride("mitkIOAdapter",
                         "mitkNrrdTbssRoiImageReader",
                         "mitk Tbss Image IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<NrrdTbssRoiVolReaderType> >::New());
}

NrrdTbssRoiImageIOFactory::~NrrdTbssRoiImageIOFactory()
{
}

const char* NrrdTbssRoiImageIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* NrrdTbssRoiImageIOFactory::GetDescription() const
{
  return "NrrdTbssImage IO Factory, allows the loading of NRRD Tbss data";
}

} // end namespace mitk
