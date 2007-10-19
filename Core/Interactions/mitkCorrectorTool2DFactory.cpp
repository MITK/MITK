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

#include "mitkCorrectorTool2DFactory.h"
#include "mitkCorrectorTool2D.h"

#include "itkVersion.h"
  
mitk::CorrectorTool2DFactory::CorrectorTool2DFactory()
{
  std::cout << "RegisterOverride" << std::endl;
  itk::ObjectFactoryBase::RegisterOverride("mitkTool",
                          "mitkCorrectorTool2D",
                          "MITK 2D region growing tool",
                          1,
                          itk::CreateObjectFunction<CorrectorTool2D>::New());
}
  
mitk::CorrectorTool2DFactory::~CorrectorTool2DFactory()
{

}

const char* mitk::CorrectorTool2DFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* mitk::CorrectorTool2DFactory::GetDescription() const
{
  return "MITK 2D region growing tool, is used by mitk::ToolManager";
}

