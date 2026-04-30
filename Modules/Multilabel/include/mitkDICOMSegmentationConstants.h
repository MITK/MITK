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
  /**
   * \brief Collection of DICOM tag path constants for DICOM Segmentation objects.
   *
   * Provides static factory methods that return DICOMTagPath instances for accessing
   * segment attributes in DICOM SEG files, including segment metadata, anatomic region
   * codes, category/type codes, and modifier sequences.
   *
   * \sa DICOMSegmentationPropertyHelper, DICOMTagPath, Label
   */
  struct MITKMULTILABEL_EXPORT DICOMSegmentationConstants
  {
    /** \brief Returns the tag path to the Segment Sequence (0062,0002). */
    static DICOMTagPath SEGMENT_SEQUENCE_PATH();

    /** \brief Returns the tag path to the Segment Number (0062,0004). */
    static DICOMTagPath SEGMENT_NUMBER_PATH();

    /** \brief Returns the sub-path to the Segment Label (0062,0005). */
    static DICOMTagPath SEGMENT_LABEL_SUB_PATH();

    /** \brief Returns the sub-path to the Segment Description (0062,0006). */
    static DICOMTagPath SEGMENT_DESCRIPTION_SUB_PATH();

    /** \brief Returns the sub-path to the Segment Algorithm Type (0062,0008). */
    static DICOMTagPath SEGMENT_ALGORITHM_TYPE_SUB_PATH();

    /** \brief Returns the sub-path to the Segment Algorithm Name (0062,0009). */
    static DICOMTagPath SEGMENT_ALGORITHM_NAME_SUB_PATH();

    /** \brief Returns the sub-path to the Tracking ID (0062,0020). */
    static DICOMTagPath SEGMENT_TRACKING_ID_SUB_PATH();

    /** \brief Returns the sub-path to the Tracking UID (0062,0021). */
    static DICOMTagPath SEGMENT_TRACKING_UID_SUB_PATH();

    /** \brief Returns the sub-path to the Anatomic Region Sequence (0008,2218). */
    static DICOMTagPath ANATOMIC_REGION_SEQUENCE_SUB_PATH();

    /** \brief Returns the sub-path to the Anatomic Region Code Value. */
    static DICOMTagPath ANATOMIC_REGION_CODE_VALUE_SUB_PATH();

    /** \brief Returns the sub-path to the Anatomic Region Coding Scheme Designator. */
    static DICOMTagPath ANATOMIC_REGION_CODE_SCHEME_SUB_PATH();

    /** \brief Returns the sub-path to the Anatomic Region Code Meaning. */
    static DICOMTagPath ANATOMIC_REGION_CODE_MEANING_SUB_PATH();

    /** \brief Returns the sub-path to the Segmented Property Category Code Sequence. */
    static DICOMTagPath SEGMENT_CATEGORY_SEQUENCE_SUB_PATH();

    /** \brief Returns the sub-path to the Category Code Value. */
    static DICOMTagPath SEGMENT_CATEGORY_CODE_VALUE_SUB_PATH();

    /** \brief Returns the sub-path to the Category Coding Scheme Designator. */
    static DICOMTagPath SEGMENT_CATEGORY_CODE_SCHEME_SUB_PATH();

    /** \brief Returns the sub-path to the Category Code Meaning. */
    static DICOMTagPath SEGMENT_CATEGORY_CODE_MEANING_SUB_PATH();

    /** \brief Returns the sub-path to the Segmented Property Type Code Sequence. */
    static DICOMTagPath SEGMENT_TYPE_SEQUENCE_SUB_PATH();

    /** \brief Returns the sub-path to the Type Code Value. */
    static DICOMTagPath SEGMENT_TYPE_CODE_VALUE_SUB_PATH();

    /** \brief Returns the sub-path to the Type Coding Scheme Designator. */
    static DICOMTagPath SEGMENT_TYPE_CODE_SCHEME_SUB_PATH();

    /** \brief Returns the sub-path to the Type Code Meaning. */
    static DICOMTagPath SEGMENT_TYPE_CODE_MEANING_SUB_PATH();

    /**
     * \brief Returns the sub-path to the Segment Type Modifier Sequence.
     * \param[in] index Optional item selection index. If not provided, a wildcarded
     *            path that captures any modifier sequence item is returned.
     * \return The DICOMTagPath for the modifier sequence.
     */
    static DICOMTagPath SEGMENT_TYPE_MODIFIER_SEQUENCE_SUB_PATH(const std::optional<DICOMTagPath::ItemSelectionIndex>& index = std::optional<DICOMTagPath::ItemSelectionIndex>());

    /**
     * \brief Returns the sub-path to the Modifier Code Value.
     * \param[in] index Optional item selection index for the modifier sequence.
     * \return The DICOMTagPath for the modifier code value.
     */
    static DICOMTagPath SEGMENT_TYPE_MODIFIER_CODE_VALUE_SUB_PATH(const std::optional<DICOMTagPath::ItemSelectionIndex>& index = std::optional<DICOMTagPath::ItemSelectionIndex>());

    /**
     * \brief Returns the sub-path to the Modifier Coding Scheme Designator.
     * \param[in] index Optional item selection index for the modifier sequence.
     * \return The DICOMTagPath for the modifier coding scheme.
     */
    static DICOMTagPath SEGMENT_TYPE_MODIFIER_CODE_SCHEME_SUB_PATH(const std::optional<DICOMTagPath::ItemSelectionIndex>& index = std::optional<DICOMTagPath::ItemSelectionIndex>());

    /**
     * \brief Returns the sub-path to the Modifier Code Meaning.
     * \param[in] index Optional item selection index for the modifier sequence.
     * \return The DICOMTagPath for the modifier code meaning.
     */
    static DICOMTagPath SEGMENT_TYPE_MODIFIER_CODE_MEANING_SUB_PATH(const std::optional<DICOMTagPath::ItemSelectionIndex>& index = std::optional<DICOMTagPath::ItemSelectionIndex>());

  };
}

#endif
