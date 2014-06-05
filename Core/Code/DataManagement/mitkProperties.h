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


#ifndef MITKPROPERTIES_H_HEADER_INCLUDED
#define MITKPROPERTIES_H_HEADER_INCLUDED

#include "mitkGenericProperty.h"
#include "mitkNumericTypes.h"
#include "mitkLookupTables.h"

namespace mitk {

   mitkDeclareGenericProperty(BoolProperty,bool,MITK_CORE_EXPORT);

   mitkDeclareGenericProperty(IntProperty,int,MITK_CORE_EXPORT);

   mitkDeclareGenericProperty(FloatProperty,float,MITK_CORE_EXPORT);

   mitkDeclareGenericProperty(DoubleProperty,double,MITK_CORE_EXPORT);

   mitkDeclareGenericProperty(Vector3DProperty,Vector3D,MITK_CORE_EXPORT);

   mitkDeclareGenericProperty(Point2dProperty,Point2D,MITK_CORE_EXPORT);

   mitkDeclareGenericProperty(Point3dProperty,Point3D,MITK_CORE_EXPORT);

   mitkDeclareGenericProperty(Point4dProperty,Point4D,MITK_CORE_EXPORT);

   mitkDeclareGenericProperty(Point3iProperty,Point3I,MITK_CORE_EXPORT);

   mitkDeclareGenericProperty(FloatLookupTableProperty, FloatLookupTable,MITK_CORE_EXPORT);
   mitkDeclareGenericProperty(BoolLookupTableProperty, BoolLookupTable,MITK_CORE_EXPORT);
   mitkDeclareGenericProperty(IntLookupTableProperty, IntLookupTable,MITK_CORE_EXPORT);
   mitkDeclareGenericProperty(StringLookupTableProperty, StringLookupTable,MITK_CORE_EXPORT);
   /**
   * \warning If you add more specialization of GenericProperty, you must also add these to the
   * templated GetPropertyValue() method in mitkPropertyList.cpp!
   */
} // namespace mitk
#endif /* MITKPROPERTIES_H_HEADER_INCLUDED */
