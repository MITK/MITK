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

   mitkDeclareGenericProperty(BoolProperty,bool,MITK_CORE_EXPORT);

   mitkDeclareGenericProperty(IntProperty,int,MITK_CORE_EXPORT);

   mitkDeclareGenericProperty(FloatProperty,float,MITK_CORE_EXPORT);

   mitkDeclareGenericProperty(DoubleProperty,double,MITK_CORE_EXPORT);

   mitkDeclareGenericProperty(Vector3DProperty,Vector3D,MITK_CORE_EXPORT);

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
