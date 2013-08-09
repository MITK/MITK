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

#include "mitkNrrdLabelSetImageIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkNrrdLabelSetImageReader.h"

#include "itkVersion.h"


namespace mitk
{

NrrdLabelSetImageIOFactory::NrrdLabelSetImageIOFactory()
{
  typedef unsigned char LabelSetPixelType;
  typedef NrrdLabelSetImageReader<LabelSetPixelType> NrrdLabelSetImageReaderType;
  this->RegisterOverride("mitkIOAdapter",
                         "mitkNrrdLabelSetImageReader",
                         "mitk Diffusion Image IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<NrrdLabelSetImageReaderType> >::New());
}

NrrdLabelSetImageIOFactory::~NrrdLabelSetImageIOFactory()
{
}

const char* NrrdLabelSetImageIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* NrrdLabelSetImageIOFactory::GetDescription() const
{
  return "NrrdLabelSetImage IO Factory, allows the loading of NRRD label set images";
}

} // end namespace mitk
