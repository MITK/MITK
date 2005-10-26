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


#ifndef MITKPROPERTIES_H_HEADER_INCLUDED
#define MITKPROPERTIES_H_HEADER_INCLUDED

#include "mitkGenericProperty.h"
#include "mitkVector.h"
#include <string>

namespace mitk {

   typedef GenericProperty<bool> BoolProperty;

   typedef GenericProperty<int> IntProperty;

   typedef GenericProperty<float> FloatProperty;

   typedef GenericProperty<double> DoubleProperty;

   typedef GenericProperty<Point3D> Point3dProperty;

   typedef GenericProperty<Point4D> Point4dProperty;

   typedef GenericProperty<itk::Point<int, 3> > Point3iProperty;

} // namespace mitk  

#endif /* MITKPROPERTIES_H_HEADER_INCLUDED */
