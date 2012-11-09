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
