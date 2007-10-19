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

#include "mitkAddContourToolFactory.h"
#include "mitkAddContourTool.h"

#include "itkVersion.h"
  
mitk::AddContourToolFactory::AddContourToolFactory()
{
  std::cout << "RegisterOverride" << std::endl;
  itk::ObjectFactoryBase::RegisterOverride("mitkTool",
                          "mitkAddContourTool",
                          "MITK 2D contour drawing tool (add contour)",
                          1,
                          itk::CreateObjectFunction<AddContourTool>::New());
}
  
mitk::AddContourToolFactory::~AddContourToolFactory()
{

}

const char* mitk::AddContourToolFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* mitk::AddContourToolFactory::GetDescription() const
{
  return "MITK 2D contour drawing tool (add contour), is used by mitk::ToolManager";
}

