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


#ifndef MITKPROPERTIES_H_HEADER_INCLUDED
#define MITKPROPERTIES_H_HEADER_INCLUDED

#include "mitkGenericProperty.h"
#include "mitkVector.h"
#include "mitkLookupTables.h"

namespace mitk {

   mitkSpecializeGenericProperty(BoolProperty,bool,false);

   mitkSpecializeGenericProperty(IntProperty,int,0);

   mitkSpecializeGenericProperty(FloatProperty,float,0.0f);

   mitkSpecializeGenericProperty(DoubleProperty,double,0.0);

   mitkSpecializeGenericProperty(Vector3DProperty,Vector3D,Vector3D(0.0f));

   mitkSpecializeGenericProperty(Point3dProperty,Point3D,Point3D::BaseArray::Filled(0.0f).GetDataPointer() );

   mitkSpecializeGenericProperty(Point4dProperty,Point4D,Point4D::BaseArray::Filled(0.0f).GetDataPointer() );

   mitkSpecializeGenericProperty(Point3iProperty,Point3I,Point3I::BaseArray::Filled(0).GetDataPointer() );
   
   mitkSpecializeGenericProperty(FloatLookupTableProperty,  FloatLookupTable,  FloatLookupTable());
   mitkSpecializeGenericProperty(BoolLookupTableProperty,   BoolLookupTable,   BoolLookupTable());
   mitkSpecializeGenericProperty(IntLookupTableProperty,    IntLookupTable,    IntLookupTable());
   mitkSpecializeGenericProperty(StringLookupTableProperty, StringLookupTable, StringLookupTable());
   /**
   * \warning If you add more specialization of GenericProperty, you must also add these to the
   * templated GetPropertyValue() method in mitkPropertyList.cpp!
   */
} // namespace mitk
#endif /* MITKPROPERTIES_H_HEADER_INCLUDED */
