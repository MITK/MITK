/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkEnumerationPropertySerializer.h>

#include <mitkRegEvalStyleProperty.h>
#include <mitkRegEvalWipeStyleProperty.h>
#include <mitkRegVisColorStyleProperty.h>
#include <mitkRegVisDirectionProperty.h>
#include <mitkRegVisStyleProperty.h>

#include <tinyxml2.h>

MITK_REGISTER_ENUM_SUB_SERIALIZER(RegEvalStyleProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(RegEvalWipeStyleProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(RegVisColorStyleProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(RegVisDirectionProperty);
MITK_REGISTER_ENUM_SUB_SERIALIZER(RegVisStyleProperty);
