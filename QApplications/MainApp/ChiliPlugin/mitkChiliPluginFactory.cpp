/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkChiliPluginFactory.h"
#include "itkCreateObjectFunction.h"
#include "itkVersion.h"
  
namespace mitk
{
ChiliPluginFactory::ChiliPluginFactory()
{
  this->RegisterOverride("mitk::ChiliPlugin",
                         "mitk::ChiliPluginImpl",
                         "MITK Chili Plugin",
                         1,
                         CreateObjectFunction<mitk::ChiliPluginImpl>::New());
  this->RegisterOverride("mitk::LightBoxImageReader",
                         "mitk::LightBoxImageReaderImpl",
                         "MITK Chili Plugin Lightbox Reader",
                         1,
                         CreateObjectFunction<mitk::LightBoxImageReaderImpl>::New());
  this->RegisterOverride("mitk::LightBoxResultImageWriter",
                         "mitk::LightBoxResultImageWriterImpl",
                         "MITK Chili Plugin Lightbox Writer",
                         1,
                         CreateObjectFunction<mitk::LightBoxResultImageWriterImpl>::New());
}
  
ChiliPluginFactory::~ChiliPluginFactory()
{
}

const char* ChiliPluginFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* ChiliPluginFactory::GetDescription() const
{
  return "Chili Plugin Factory, overrides stubs";
}

} // end namespace mitk

