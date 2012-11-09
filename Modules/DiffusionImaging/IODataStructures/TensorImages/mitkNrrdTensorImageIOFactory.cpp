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

#include "mitkNrrdTensorImageIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkNrrdTensorImageReader.h"

#include "itkVersion.h"


namespace mitk
{

NrrdTensorImageIOFactory::NrrdTensorImageIOFactory()
{
  typedef NrrdTensorImageReader NrrdDTIReaderType;
  this->RegisterOverride("mitkIOAdapter",
                         "mitkNrrdTensorImageReader",
                         "TensorImages IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<NrrdDTIReaderType> >::New());
}

NrrdTensorImageIOFactory::~NrrdTensorImageIOFactory()
{
}

const char* NrrdTensorImageIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* NrrdTensorImageIOFactory::GetDescription() const
{
  return "NrrdTensorImage IO Factory, allows the loading of NRRD tensor data";
}

} // end namespace mitk
