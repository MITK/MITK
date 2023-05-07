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

#include <MitkMultilabelExports.h>


namespace mitk
{
  struct MITKMULTILABEL_EXPORT DICOMSegmentationConstants
  {
    static DICOMTagPath SEGMENT_SEQUENCE_PATH();
    static DICOMTagPath SEGMENT_NUMBER_PATH();
    static DICOMTagPath SEGMENT_LABEL_PATH();
    static DICOMTagPath SEGMENT_ALGORITHM_TYPE_PATH();

    static DICOMTagPath ANATOMIC_REGION_SEQUENCE_PATH();
    static DICOMTagPath ANATOMIC_REGION_CODE_VALUE_PATH();
    static DICOMTagPath ANATOMIC_REGION_CODE_SCHEME_PATH();
    static DICOMTagPath ANATOMIC_REGION_CODE_MEANING_PATH();

    static DICOMTagPath SEGMENTED_PROPERTY_CATEGORY_SEQUENCE_PATH();
    static DICOMTagPath SEGMENT_CATEGORY_CODE_VALUE_PATH();
    static DICOMTagPath SEGMENT_CATEGORY_CODE_SCHEME_PATH();
    static DICOMTagPath SEGMENT_CATEGORY_CODE_MEANING_PATH();

    static DICOMTagPath SEGMENTED_PROPERTY_TYPE_SEQUENCE_PATH();
    static DICOMTagPath SEGMENT_TYPE_CODE_VALUE_PATH();
    static DICOMTagPath SEGMENT_TYPE_CODE_SCHEME_PATH();
    static DICOMTagPath SEGMENT_TYPE_CODE_MEANING_PATH();

    static DICOMTagPath SEGMENTED_PROPERTY_MODIFIER_SEQUENCE_PATH();
    static DICOMTagPath SEGMENT_MODIFIER_CODE_VALUE_PATH();
    static DICOMTagPath SEGMENT_MODIFIER_CODE_SCHEME_PATH();
    static DICOMTagPath SEGMENT_MODIFIER_CODE_MEANING_PATH();


  };
}

#endif
