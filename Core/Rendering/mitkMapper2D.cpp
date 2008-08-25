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


#include "mitkMapper2D.h"

mitk::Mapper2D::Mapper2D()
{
}

mitk::Mapper2D::~Mapper2D()
{
}

void mitk::Mapper2D::SetGeometry3D(const mitk::Geometry3D* aGeometry3D)
{
    m_Geometry3D = aGeometry3D;
    Modified();
}
