/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMSegmentationConstants_h
#define mitkDICOMSegmentationConstants_h

#include <mitkDICOMTagPath.h>
#include <optional>

#include <MitkMultilabelExports.h>


namespace mitk
{
  struct MITKMULTILABEL_EXPORT DICOMSegmentationConstants
  {
    static DICOMTagPath SEGMENT_SEQUENCE_PATH();
    static DICOMTagPath SEGMENT_NUMBER_PATH();
    static DICOMTagPath SEGMENT_LABEL_SUB_PATH();
    static DICOMTagPath SEGMENT_DESCRIPTION_SUB_PATH();
    static DICOMTagPath SEGMENT_ALGORITHM_TYPE_SUB_PATH();
    static DICOMTagPath SEGMENT_ALGORITHM_NAME_SUB_PATH();
    static DICOMTagPath SEGMENT_TRACKING_ID_SUB_PATH();
    static DICOMTagPath SEGMENT_TRACKING_UID_SUB_PATH();

    static DICOMTagPath ANATOMIC_REGION_SEQUENCE_SUB_PATH();
    static DICOMTagPath ANATOMIC_REGION_CODE_VALUE_SUB_PATH();
    static DICOMTagPath ANATOMIC_REGION_CODE_SCHEME_SUB_PATH();
    static DICOMTagPath ANATOMIC_REGION_CODE_MEANING_SUB_PATH();

    static DICOMTagPath SEGMENT_CATEGORY_SEQUENCE_SUB_PATH();
    static DICOMTagPath SEGMENT_CATEGORY_CODE_VALUE_SUB_PATH();
    static DICOMTagPath SEGMENT_CATEGORY_CODE_SCHEME_SUB_PATH();
    static DICOMTagPath SEGMENT_CATEGORY_CODE_MEANING_SUB_PATH();

    static DICOMTagPath SEGMENT_TYPE_SEQUENCE_SUB_PATH();
    static DICOMTagPath SEGMENT_TYPE_CODE_VALUE_SUB_PATH();
    static DICOMTagPath SEGMENT_TYPE_CODE_SCHEME_SUB_PATH();
    static DICOMTagPath SEGMENT_TYPE_CODE_MEANING_SUB_PATH();

    /** DICOM tag sub path for the modifier sequence. If no index is provided, a wildcarded path will be generated
    that would capture any modifier sequence.*/
    static DICOMTagPath SEGMENT_TYPE_MODIFIER_SEQUENCE_SUB_PATH(const std::optional<DICOMTagPath::ItemSelectionIndex>& index = std::optional<DICOMTagPath::ItemSelectionIndex>());
    static DICOMTagPath SEGMENT_TYPE_MODIFIER_CODE_VALUE_SUB_PATH(const std::optional<DICOMTagPath::ItemSelectionIndex>& index = std::optional<DICOMTagPath::ItemSelectionIndex>());
    static DICOMTagPath SEGMENT_TYPE_MODIFIER_CODE_SCHEME_SUB_PATH(const std::optional<DICOMTagPath::ItemSelectionIndex>& index = std::optional<DICOMTagPath::ItemSelectionIndex>());
    static DICOMTagPath SEGMENT_TYPE_MODIFIER_CODE_MEANING_SUB_PATH(const std::optional<DICOMTagPath::ItemSelectionIndex>& index = std::optional<DICOMTagPath::ItemSelectionIndex>());

  };
}

#endif
