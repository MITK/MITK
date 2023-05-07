/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkProperties_h
#define mitkProperties_h

#include "mitkGenericProperty.h"
#include "mitkLookupTables.h"
#include "mitkNumericTypes.h"

namespace mitk
{
  mitkDeclareGenericProperty(BoolProperty, bool, MITKCORE_EXPORT);

  mitkDeclareGenericProperty(IntProperty, int, MITKCORE_EXPORT);

  mitkDeclareGenericProperty(UIntProperty, unsigned int, MITKCORE_EXPORT);

  mitkDeclareGenericProperty(UShortProperty, unsigned short, MITKCORE_EXPORT);

  mitkDeclareGenericProperty(FloatProperty, float, MITKCORE_EXPORT);

  mitkDeclareGenericProperty(DoubleProperty, double, MITKCORE_EXPORT);

  mitkDeclareGenericProperty(Vector3DProperty, Vector3D, MITKCORE_EXPORT);

  mitkDeclareGenericProperty(Point2dProperty, Point2D, MITKCORE_EXPORT);

  mitkDeclareGenericProperty(Point3dProperty, Point3D, MITKCORE_EXPORT);

  mitkDeclareGenericProperty(Point4dProperty, Point4D, MITKCORE_EXPORT);

  mitkDeclareGenericProperty(Point3iProperty, Point3I, MITKCORE_EXPORT);

  mitkDeclareGenericProperty(FloatLookupTableProperty, FloatLookupTable, MITKCORE_EXPORT);
  mitkDeclareGenericProperty(BoolLookupTableProperty, BoolLookupTable, MITKCORE_EXPORT);
  mitkDeclareGenericProperty(IntLookupTableProperty, IntLookupTable, MITKCORE_EXPORT);
  mitkDeclareGenericProperty(StringLookupTableProperty, StringLookupTable, MITKCORE_EXPORT);
  /**
  * \warning If you add more specialization of GenericProperty, you must also add these to the
  * templated GetPropertyValue() method in mitkPropertyList.cpp!
  */
} // namespace mitk
#endif
