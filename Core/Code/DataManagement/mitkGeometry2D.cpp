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

//Deprecated Class
#include "mitkGeometry2D.h"

// Standard constructor for the New() macro. Sets the geometry to 3 dimensions
mitk::Geometry2D::Geometry2D() :  PlaneGeometry()
{
}
mitk::Geometry2D::Geometry2D(const Geometry2D& other) : PlaneGeometry(other)
{
}

mitk::Geometry2D::~Geometry2D()
{
}
