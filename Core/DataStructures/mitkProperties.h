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
#include "mitkTransferFunction.h"
#include <string>

namespace mitk {

   typedef GenericProperty<bool> BoolProperty;
   template class GenericProperty<bool>;

   typedef GenericProperty<int> IntProperty;
   template class GenericProperty<int>;

   typedef GenericProperty<float> FloatProperty;
   template class GenericProperty<float>;

   typedef GenericProperty<double> DoubleProperty;
   template class GenericProperty<double>;

   typedef GenericProperty<Point3D> Point3dProperty;
   template class GenericProperty<Point3D>;

   typedef GenericProperty<itk::Point<int, 3> > Point3iProperty;
   template class GenericProperty<itk::Point<int, 3> >;
    
   typedef GenericProperty<mitk::TransferFunction::Pointer> TransferFunctionProperty;
} // namespace mitk  

#endif /* MITKPROPERTIES_H_HEADER_INCLUDED */
