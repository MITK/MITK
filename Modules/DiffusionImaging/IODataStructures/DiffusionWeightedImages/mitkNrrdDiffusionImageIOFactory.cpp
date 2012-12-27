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

#include "mitkNrrdDiffusionImageIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkNrrdDiffusionImageReader.h"

#include "itkVersion.h"


namespace mitk
{

NrrdDiffusionImageIOFactory::NrrdDiffusionImageIOFactory()
{
  typedef short DiffusionPixelType;
  typedef itk::VectorImage< DiffusionPixelType, 3 >   DiffusionImageType;
  typedef NrrdDiffusionImageReader<DiffusionPixelType> NrrdDiffVolReaderType;
  this->RegisterOverride("mitkIOAdapter",
                         "mitkNrrdDiffusionImageReader",
                         "mitk Diffusion Image IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<NrrdDiffVolReaderType> >::New());
}

NrrdDiffusionImageIOFactory::~NrrdDiffusionImageIOFactory()
{
}

const char* NrrdDiffusionImageIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* NrrdDiffusionImageIOFactory::GetDescription() const
{
  return "NrrdDiffusionImage IO Factory, allows the loading of NRRD DWI data";
}

} // end namespace mitk
