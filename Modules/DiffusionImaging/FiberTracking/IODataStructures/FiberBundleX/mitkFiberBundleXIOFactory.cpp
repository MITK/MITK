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
