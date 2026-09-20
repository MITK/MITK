/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkEnumerationPropertySerializer.h>

#include <mitkModalityProperty.h>
#include <mitkPlaneOrientationProperty.h>
#include <mitkPointSetShapeProperty.h>
#include <mitkRenderingModeProperty.h>
#include <mitkVtkInterpolationProperty.h>
#include <mitkVtkRepresentationProperty.h>
#include <mitkVtkResliceInterpolationProperty.h>
#include <mitkVtkScalarModeProperty.h>

#include <tinyxml2.h>

MITK_REGISTER_ENUM_SUB_SERIALIZER(PlaneOrientationProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(VtkInterpolationProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(VtkRepresentationProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(VtkResliceInterpolationProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(VtkScalarModeProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(ModalityProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(RenderingModeProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(PointSetShapeProperty);
