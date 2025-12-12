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

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_LABEL_SUB_PATH()
  {
    return DICOMTagPath(0x0062, 0x0005);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_DESCRIPTION_SUB_PATH()
  {
    return DICOMTagPath(0x0062, 0x0006);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_ALGORITHM_TYPE_SUB_PATH()
  {
    return DICOMTagPath(0x0062, 0x0008);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_ALGORITHM_NAME_SUB_PATH()
  {
    return DICOMTagPath(0x0062, 0x0009);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_TRACKING_ID_SUB_PATH()
  {
    return DICOMTagPath(0x0062, 0x0020);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_TRACKING_UID_SUB_PATH()
  {
    return DICOMTagPath(0x0062, 0x0021);
  }

  DICOMTagPath DICOMSegmentationConstants::ANATOMIC_REGION_SEQUENCE_SUB_PATH()
  {
    DICOMTagPath result;
    result.AddSelection(0x0008, 0x2218, 1); //this sequence has always only one item, therefor preselect directly
    return result;
  }

  DICOMTagPath DICOMSegmentationConstants::ANATOMIC_REGION_CODE_VALUE_SUB_PATH()
  {
    return ANATOMIC_REGION_SEQUENCE_SUB_PATH().AddElement(0x008, 0x0100);
  }

  DICOMTagPath DICOMSegmentationConstants::ANATOMIC_REGION_CODE_SCHEME_SUB_PATH()
  {
    return ANATOMIC_REGION_SEQUENCE_SUB_PATH().AddElement(0x008, 0x0102);
  }

  DICOMTagPath DICOMSegmentationConstants::ANATOMIC_REGION_CODE_MEANING_SUB_PATH()
  {
    return ANATOMIC_REGION_SEQUENCE_SUB_PATH().AddElement(0x008, 0x0104);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_CATEGORY_SEQUENCE_SUB_PATH()
  {
    DICOMTagPath result;
    result.AddSelection(0x0062, 0x0003, 1); //this sequence has always only one item, therefor preselect directly
    return result;
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_VALUE_SUB_PATH()
  {
    return SEGMENT_CATEGORY_SEQUENCE_SUB_PATH().AddElement(0x008, 0x0100);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_SCHEME_SUB_PATH()
  {
    return SEGMENT_CATEGORY_SEQUENCE_SUB_PATH().AddElement(0x008, 0x0102);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_MEANING_SUB_PATH()
  {
    return SEGMENT_CATEGORY_SEQUENCE_SUB_PATH().AddElement(0x008, 0x0104);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_TYPE_SEQUENCE_SUB_PATH()
  {
    DICOMTagPath result;
    result.AddSelection(0x0062, 0x000F, 1); //this sequence has always only one item, therefor preselect directly
    return result;
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_TYPE_CODE_VALUE_SUB_PATH()
  {
    return SEGMENT_TYPE_SEQUENCE_SUB_PATH().AddElement(0x008, 0x0100);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_TYPE_CODE_SCHEME_SUB_PATH()
  {
    return SEGMENT_TYPE_SEQUENCE_SUB_PATH().AddElement(0x008, 0x0102);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_TYPE_CODE_MEANING_SUB_PATH()
  {
    return SEGMENT_TYPE_SEQUENCE_SUB_PATH().AddElement(0x008, 0x0104);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_TYPE_MODIFIER_SEQUENCE_SUB_PATH(const std::optional<DICOMTagPath::ItemSelectionIndex>& index)
  {
    if (index.has_value())
      return DICOMTagPath().AddSelection(0x0062, 0x0011, index.value());

    return DICOMTagPath().AddAnySelection(0x0062, 0x0011);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_TYPE_MODIFIER_CODE_VALUE_SUB_PATH(const std::optional<DICOMTagPath::ItemSelectionIndex>& index)
  {
    return DICOMSegmentationConstants::SEGMENT_TYPE_MODIFIER_SEQUENCE_SUB_PATH(index).AddElement(0x008, 0x0100);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_TYPE_MODIFIER_CODE_SCHEME_SUB_PATH(const std::optional<DICOMTagPath::ItemSelectionIndex>& index)
  {
    return DICOMSegmentationConstants::SEGMENT_TYPE_MODIFIER_SEQUENCE_SUB_PATH(index).AddElement(0x008, 0x0102);
  }

  DICOMTagPath DICOMSegmentationConstants::SEGMENT_TYPE_MODIFIER_CODE_MEANING_SUB_PATH(const std::optional<DICOMTagPath::ItemSelectionIndex>& index)
  {
    return DICOMSegmentationConstants::SEGMENT_TYPE_MODIFIER_SEQUENCE_SUB_PATH(index).AddElement(0x008, 0x0104);
  }

}
