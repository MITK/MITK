/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkProperties.h"

mitkDefineGenericProperty(BoolProperty, bool, false);

mitkDefineGenericProperty(IntProperty, int, 0);

mitkDefineGenericProperty(UIntProperty, unsigned int, 0);

mitkDefineGenericProperty(UShortProperty, unsigned short, 0);

mitkDefineGenericProperty(FloatProperty, float, 0.0f);

mitkDefineGenericProperty(DoubleProperty, double, 0.0);

mitkDefineGenericProperty(Vector3DProperty, Vector3D, Vector3D(0.0f));

mitkDefineGenericProperty(Point2dProperty, Point2D, Point2D::BaseArray::Filled(0.0f).GetDataPointer());

mitkDefineGenericProperty(Point3dProperty, Point3D, Point3D::BaseArray::Filled(0.0f).GetDataPointer());

mitkDefineGenericProperty(Point4dProperty, Point4D, Point4D::BaseArray::Filled(0.0f).GetDataPointer());

mitkDefineGenericProperty(Point3iProperty, Point3I, Point3I::BaseArray::Filled(0).GetDataPointer());

mitkDefineGenericProperty(FloatLookupTableProperty, FloatLookupTable, FloatLookupTable());
mitkDefineGenericProperty(BoolLookupTableProperty, BoolLookupTable, BoolLookupTable());
mitkDefineGenericProperty(IntLookupTableProperty, IntLookupTable, IntLookupTable());
mitkDefineGenericProperty(StringLookupTableProperty, StringLookupTable, StringLookupTable());
