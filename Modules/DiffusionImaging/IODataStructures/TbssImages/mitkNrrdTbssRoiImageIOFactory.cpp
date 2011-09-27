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
