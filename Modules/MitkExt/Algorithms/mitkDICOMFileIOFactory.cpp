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

#include "mitkDICOMFileIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkDICOMFileReader.h"

#include "itkVersion.h"

  
namespace mitk
{
DICOMFileIOFactory::DICOMFileIOFactory()
{
  this->RegisterOverride("mitkIOAdapter",
                         "mitkDICOMFileReader",
                         "mitk DICOM Image IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<DICOMFileReader> >::New());
}
  
DICOMFileIOFactory::~DICOMFileIOFactory()
{
}

const char* DICOMFileIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* DICOMFileIOFactory::GetDescription() const
{
  return "DICOMFile IO Factory, allows the loading of DICOM files";
}

} // end namespace mitk
