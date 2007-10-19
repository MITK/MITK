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

#include "mitkEraseRegionToolFactory.h"
#include "mitkEraseRegionTool.h"

#include "itkVersion.h"
  
mitk::EraseRegionToolFactory::EraseRegionToolFactory()
{
  std::cout << "RegisterOverride" << std::endl;
  itk::ObjectFactoryBase::RegisterOverride("mitkTool",
                          "mitkEraseRegionTool",
                          "MITK 2D contour drawing tool (add contour)",
                          1,
                          itk::CreateObjectFunction<EraseRegionTool>::New());
}
  
mitk::EraseRegionToolFactory::~EraseRegionToolFactory()
{

}

const char* mitk::EraseRegionToolFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* mitk::EraseRegionToolFactory::GetDescription() const
{
  return "MITK 2D region filling tool, is used by mitk::ToolManager";
}

