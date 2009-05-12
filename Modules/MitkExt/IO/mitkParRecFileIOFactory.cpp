/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A ParRecTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkParRecFileIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkParRecFileReader.h"

#include "itkVersion.h"

  
namespace mitk
{
ParRecFileIOFactory::ParRecFileIOFactory()
{
  this->RegisterOverride("mitkIOAdapter",
                         "mitkParRecFileReader",
                         "mitk ParRec Image IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<ParRecFileReader> >::New());
}
  
ParRecFileIOFactory::~ParRecFileIOFactory()
{
}

const char* ParRecFileIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* ParRecFileIOFactory::GetDescription() const
{
  return "ParRecFile IO Factory, allows the loading of ParRec images";
}

} // end namespace mitk
