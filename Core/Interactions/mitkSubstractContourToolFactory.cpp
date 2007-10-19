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

#include "mitkSubstractContourToolFactory.h"
#include "mitkSubstractContourTool.h"

#include "itkVersion.h"
  
mitk::SubstractContourToolFactory::SubstractContourToolFactory()
{
  std::cout << "RegisterOverride" << std::endl;
  itk::ObjectFactoryBase::RegisterOverride("mitkTool",
                          "mitkSubstractContourTool",
                          "MITK 2D contour drawing tool (add contour)",
                          1,
                          itk::CreateObjectFunction<SubstractContourTool>::New());
}
  
mitk::SubstractContourToolFactory::~SubstractContourToolFactory()
{

}

const char* mitk::SubstractContourToolFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* mitk::SubstractContourToolFactory::GetDescription() const
{
  return "MITK 2D contour drawing tool (add contour), is used by mitk::ToolManager";
}

