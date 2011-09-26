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
