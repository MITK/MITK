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

#include "mitkDICOMSegmentationConstants.h"

namespace mitk
{
  DICOMTagPath DICOMSegmentationConstants::SEGMENT_SEQUENCE_PATH()
  {
    static DICOMTagPath path = DICOMTagPath().AddElement(0x0062, 0x0002);
    return path;
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_NUMBER_PATH()
  {
    static DICOMTagPath path =
      DICOMTagPath(DICOMSegmentationConstants::SEGMENT_SEQUENCE_PATH()).AddElement(0x0062, 0x0004);
    return path;
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_LABEL_PATH()
  {
    static DICOMTagPath path =
      DICOMTagPath(DICOMSegmentationConstants::SEGMENT_SEQUENCE_PATH()).AddElement(0x0062, 0x0005);
    return path;
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_ALGORITHM_TYPE_PATH()
  {
    static DICOMTagPath path =
      DICOMTagPath(DICOMSegmentationConstants::SEGMENT_SEQUENCE_PATH()).AddElement(0x0062, 0x0008);
    return path;
  }

  DICOMTagPath DICOMSegmentationConstants::ANATOMIC_REGION_SEQUENCE_PATH()
  {
    static DICOMTagPath path =
      DICOMTagPath(DICOMSegmentationConstants::SEGMENT_SEQUENCE_PATH()).AddElement(0x0008, 0x2218);
    return path;
  }

  DICOMTagPath DICOMSegmentationConstants::ANATOMIC_REGION_CODE_VALUE_PATH()
  {
    static DICOMTagPath path =
      DICOMTagPath(DICOMSegmentationConstants::ANATOMIC_REGION_SEQUENCE_PATH()).AddElement(0x008, 0x0100);
    return path;
  }

  DICOMTagPath DICOMSegmentationConstants::ANATOMIC_REGION_CODE_SCHEME_PATH()
  {
    static DICOMTagPath path =
      DICOMTagPath(DICOMSegmentationConstants::ANATOMIC_REGION_SEQUENCE_PATH()).AddElement(0x008, 0x0102);
    return path;
  }

  DICOMTagPath DICOMSegmentationConstants::ANATOMIC_REGION_CODE_MEANING_PATH()
  {
    static DICOMTagPath path =
      DICOMTagPath(DICOMSegmentationConstants::ANATOMIC_REGION_SEQUENCE_PATH()).AddElement(0x008, 0x0104);
    return path;
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENTED_PROPERTY_CATEGORY_SEQUENCE_PATH()
  {
    static DICOMTagPath path =
      DICOMTagPath(DICOMSegmentationConstants::SEGMENT_SEQUENCE_PATH()).AddElement(0x0062, 0x0003);
    return path;
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_VALUE_PATH()
  {
    static DICOMTagPath path =
      DICOMTagPath(DICOMSegmentationConstants::SEGMENTED_PROPERTY_CATEGORY_SEQUENCE_PATH()).AddElement(0x008, 0x0100);
    return path;
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_SCHEME_PATH()
  {
    static DICOMTagPath path =
      DICOMTagPath(DICOMSegmentationConstants::SEGMENTED_PROPERTY_CATEGORY_SEQUENCE_PATH()).AddElement(0x008, 0x0102);
    return path;
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_MEANING_PATH()
  {
    static DICOMTagPath path =
      DICOMTagPath(DICOMSegmentationConstants::SEGMENTED_PROPERTY_CATEGORY_SEQUENCE_PATH()).AddElement(0x008, 0x0104);
    return path;
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENTED_PROPERTY_TYPE_SEQUENCE_PATH()
  {
    static DICOMTagPath path =
      DICOMTagPath(DICOMSegmentationConstants::SEGMENT_SEQUENCE_PATH()).AddElement(0x0062, 0x000F);
    return path;
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_TYPE_CODE_VALUE_PATH()
  {
    static DICOMTagPath path =
      DICOMTagPath(DICOMSegmentationConstants::SEGMENTED_PROPERTY_TYPE_SEQUENCE_PATH()).AddElement(0x008, 0x0100);
    return path;
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_TYPE_CODE_SCHEME_PATH()
  {
    static DICOMTagPath path =
      DICOMTagPath(DICOMSegmentationConstants::SEGMENTED_PROPERTY_TYPE_SEQUENCE_PATH()).AddElement(0x008, 0x0102);
    return path;
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_TYPE_CODE_MEANING_PATH()
  {
    static DICOMTagPath path =
      DICOMTagPath(DICOMSegmentationConstants::SEGMENTED_PROPERTY_TYPE_SEQUENCE_PATH()).AddElement(0x008, 0x0104);
    return path;
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENTED_PROPERTY_MODIFIER_SEQUENCE_PATH()
  {
    static DICOMTagPath path =
      DICOMTagPath(DICOMSegmentationConstants::SEGMENTED_PROPERTY_TYPE_SEQUENCE_PATH()).AddElement(0x0062, 0x0011);
    return path;
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_VALUE_PATH()
  {
    static DICOMTagPath path =
      DICOMTagPath(DICOMSegmentationConstants::SEGMENTED_PROPERTY_MODIFIER_SEQUENCE_PATH()).AddElement(0x008, 0x0100);
    return path;
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_SCHEME_PATH()
  {
    static DICOMTagPath path =
      DICOMTagPath(DICOMSegmentationConstants::SEGMENTED_PROPERTY_MODIFIER_SEQUENCE_PATH()).AddElement(0x008, 0x0102);
    return path;
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_MEANING_PATH()
  {
    static DICOMTagPath path =
      DICOMTagPath(DICOMSegmentationConstants::SEGMENTED_PROPERTY_MODIFIER_SEQUENCE_PATH()).AddElement(0x008, 0x0104);
    return path;
  }
}
