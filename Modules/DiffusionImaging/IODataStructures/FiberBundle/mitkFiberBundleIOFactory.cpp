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

#include "mitkFiberBundleIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkFiberBundleReader.h"

#include "itkVersion.h"

//NOTE>umbenennen in FBReaderType

  
namespace mitk
{

FiberBundleIOFactory::FiberBundleIOFactory()
{
  typedef FiberBundleReader FiberBundleReaderType;
  this->RegisterOverride("mitkIOAdapter", //beibehalten
                         "mitkFiberBundleReader", //umbenennen
                         "Fiber Bundle IO", //angezeigter name
                         1,
                         itk::CreateObjectFunction<IOAdapter<FiberBundleReaderType> >::New());
}
  
FiberBundleIOFactory::~FiberBundleIOFactory()
{
}

const char* FiberBundleIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* FiberBundleIOFactory::GetDescription() const
{
  return "FibreBundleIOFactory, allows the loading of FibreBundles";
}

} // end namespace mitk
