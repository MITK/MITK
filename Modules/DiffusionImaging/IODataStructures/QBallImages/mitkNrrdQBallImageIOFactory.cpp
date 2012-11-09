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

#include "mitkNrrdQBallImageIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkNrrdQBallImageReader.h"

#include "itkVersion.h"


namespace mitk
{

NrrdQBallImageIOFactory::NrrdQBallImageIOFactory()
{
  typedef NrrdQBallImageReader NrrdQBIReaderType;
  this->RegisterOverride("mitkIOAdapter",
                         "mitkNrrdQBallImageReader",
                         "QBallImages IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<NrrdQBIReaderType> >::New());
}

NrrdQBallImageIOFactory::~NrrdQBallImageIOFactory()
{
}

const char* NrrdQBallImageIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* NrrdQBallImageIOFactory::GetDescription() const
{
  return "NrrdQBallImage IO Factory, allows the loading of NRRD qball data";
}

} // end namespace mitk
