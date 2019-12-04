/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMSegmentationConstants.h"

namespace mitk
{
  DICOMTagPath DICOMSegmentationConstants::SEGMENT_SEQUENCE_PATH()
  {
    return DICOMTagPath().AddElement(0x0062, 0x0002);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_NUMBER_PATH()
  {
    return DICOMSegmentationConstants::SEGMENT_SEQUENCE_PATH().AddElement(0x0062, 0x0004);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_LABEL_PATH()
  {
    return DICOMSegmentationConstants::SEGMENT_SEQUENCE_PATH().AddElement(0x0062, 0x0005);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_ALGORITHM_TYPE_PATH()
  {
    return DICOMSegmentationConstants::SEGMENT_SEQUENCE_PATH().AddElement(0x0062, 0x0008);
  }

  DICOMTagPath DICOMSegmentationConstants::ANATOMIC_REGION_SEQUENCE_PATH()
  {
    return DICOMSegmentationConstants::SEGMENT_SEQUENCE_PATH().AddElement(0x0008, 0x2218);
  }

  DICOMTagPath DICOMSegmentationConstants::ANATOMIC_REGION_CODE_VALUE_PATH()
  {
    return DICOMSegmentationConstants::ANATOMIC_REGION_SEQUENCE_PATH().AddElement(0x008, 0x0100);
  }

  DICOMTagPath DICOMSegmentationConstants::ANATOMIC_REGION_CODE_SCHEME_PATH()
  {
    return DICOMSegmentationConstants::ANATOMIC_REGION_SEQUENCE_PATH().AddElement(0x008, 0x0102);
  }

  DICOMTagPath DICOMSegmentationConstants::ANATOMIC_REGION_CODE_MEANING_PATH()
  {
    return DICOMSegmentationConstants::ANATOMIC_REGION_SEQUENCE_PATH().AddElement(0x008, 0x0104);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENTED_PROPERTY_CATEGORY_SEQUENCE_PATH()
  {
    return DICOMSegmentationConstants::SEGMENT_SEQUENCE_PATH().AddElement(0x0062, 0x0003);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_VALUE_PATH()
  {
    return DICOMSegmentationConstants::SEGMENTED_PROPERTY_CATEGORY_SEQUENCE_PATH().AddElement(0x008, 0x0100);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_SCHEME_PATH()
  {
    return DICOMSegmentationConstants::SEGMENTED_PROPERTY_CATEGORY_SEQUENCE_PATH().AddElement(0x008, 0x0102);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_MEANING_PATH()
  {
    return DICOMSegmentationConstants::SEGMENTED_PROPERTY_CATEGORY_SEQUENCE_PATH().AddElement(0x008, 0x0104);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENTED_PROPERTY_TYPE_SEQUENCE_PATH()
  {
    return DICOMSegmentationConstants::SEGMENT_SEQUENCE_PATH().AddElement(0x0062, 0x000F);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_TYPE_CODE_VALUE_PATH()
  {
    return DICOMSegmentationConstants::SEGMENTED_PROPERTY_TYPE_SEQUENCE_PATH().AddElement(0x008, 0x0100);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_TYPE_CODE_SCHEME_PATH()
  {
    return DICOMSegmentationConstants::SEGMENTED_PROPERTY_TYPE_SEQUENCE_PATH().AddElement(0x008, 0x0102);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_TYPE_CODE_MEANING_PATH()
  {
    return DICOMSegmentationConstants::SEGMENTED_PROPERTY_TYPE_SEQUENCE_PATH().AddElement(0x008, 0x0104);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENTED_PROPERTY_MODIFIER_SEQUENCE_PATH()
  {
    return DICOMSegmentationConstants::SEGMENTED_PROPERTY_TYPE_SEQUENCE_PATH().AddElement(0x0062, 0x0011);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_VALUE_PATH()
  {
    return DICOMSegmentationConstants::SEGMENTED_PROPERTY_MODIFIER_SEQUENCE_PATH().AddElement(0x008, 0x0100);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_SCHEME_PATH()
  {
    return DICOMSegmentationConstants::SEGMENTED_PROPERTY_MODIFIER_SEQUENCE_PATH().AddElement(0x008, 0x0102);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_MEANING_PATH()
  {
    return DICOMSegmentationConstants::SEGMENTED_PROPERTY_MODIFIER_SEQUENCE_PATH().AddElement(0x008, 0x0104);
  }
}
