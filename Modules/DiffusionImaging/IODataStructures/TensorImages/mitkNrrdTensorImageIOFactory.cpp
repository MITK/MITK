/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 14:46:19 +0100 (Di, 11 Dez 2007) $
Version:   $Revision: 6607 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
