/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "mitkCuboidObjectCutter.h"
#include "mitkCuboidObjectCutter.txx"

#include "mitkImageAccessByItk.h"

namespace mitk
{

CuboidObjectCutter::CuboidObjectCutter()
{
}

CuboidObjectCutter::~CuboidObjectCutter()
{
}

void CuboidObjectCutter::ComputeData(mitk::Image* input3D, int boTimeStep)
{
  AccessFixedDimensionByItk_2(input3D, CuboidObjectCutter::CutImage, 3, this, boTimeStep);
}

}
