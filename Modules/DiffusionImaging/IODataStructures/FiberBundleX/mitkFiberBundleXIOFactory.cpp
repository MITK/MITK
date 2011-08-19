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

#include "mitkFiberBundleXIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkFiberBundleXReader.h"

#include "itkVersion.h"

//NOTE>umbenennen in FBReaderType

  
namespace mitk
{

FiberBundleXIOFactory::FiberBundleXIOFactory()
{
  typedef FiberBundleXReader FiberBundleXReaderType;
  this->RegisterOverride("mitkIOAdapter", //beibehalten
                         "mitkFiberBundleXReader", //umbenennen
                         "Fiber Bundle IO", //angezeigter name
                         1,
                         itk::CreateObjectFunction<IOAdapter<FiberBundleXReaderType> >::New());
}
  
FiberBundleXIOFactory::~FiberBundleXIOFactory()
{
}

const char* FiberBundleXIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* FiberBundleXIOFactory::GetDescription() const
{
  return "FibreBundleIOFactory, allows the loading of FibreBundles";
}

} // end namespace mitk
