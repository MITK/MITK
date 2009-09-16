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
