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

#include "mitkFillRegionToolFactory.h"
#include "mitkFillRegionTool.h"

#include "itkVersion.h"
  
mitk::FillRegionToolFactory::FillRegionToolFactory()
{
  std::cout << "RegisterOverride" << std::endl;
  itk::ObjectFactoryBase::RegisterOverride("mitkTool",
                          "mitkFillRegionTool",
                          "MITK 2D contour drawing tool (add contour)",
                          1,
                          itk::CreateObjectFunction<FillRegionTool>::New());
}
  
mitk::FillRegionToolFactory::~FillRegionToolFactory()
{

}

const char* mitk::FillRegionToolFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* mitk::FillRegionToolFactory::GetDescription() const
{
  return "MITK 2D region filling tool, is used by mitk::ToolManager";
}

