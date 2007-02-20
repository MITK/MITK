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

namespace mitk {

   mitkSpecializeGenericProperty(BoolProperty,bool);
   
   mitkSpecializeGenericProperty(IntProperty,int);
   
   mitkSpecializeGenericProperty(FloatProperty,float);
   
   mitkSpecializeGenericProperty(DoubleProperty,double);
   
   mitkSpecializeGenericProperty(Vector3DProperty,Vector3D);
   
   mitkSpecializeGenericProperty(Point3dProperty,Point3D);
   
   mitkSpecializeGenericProperty(Point4dProperty,Point4D);
   
   mitkSpecializeGenericProperty(Point3iProperty,Point3I);
   
} // namespace mitk  

#endif /* MITKPROPERTIES_H_HEADER_INCLUDED */
