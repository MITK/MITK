/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMSegmentationIO_h
#define mitkDICOMSegmentationIO_h

#include <mitkAbstractFileIO.h>
#include <mitkLabelSetImage.h>

#include <mitkDICOMTagsOfInterestAddHelper.h>

#include <dcmqi/Dicom2ItkConverterBase.h>
#include <dcmqi/JSONSegmentationMetaInformationHandler.h>

#include <dcmtk/dcmdata/dcitem.h>

#include <memory>
#include <vector>

namespace mitk
{
  /**
   * \brief Read and write MultiLabelSegmentation objects as DICOM Segmentation files.
   * \ingroup Process
   */
  class DICOMSegmentationIO : public mitk::AbstractFileIO
  {
  public:
    typedef mitk::MultiLabelSegmentation InputType;
    typedef itk::Image<unsigned short, 3> itkInputImageType;
    typedef itk::Image<short, 3> itkInternalImageType;

    /** \brief Default constructor. Registers reader/writer for the DICOM Segmentation MIME type. */
    DICOMSegmentationIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    /** \brief Return the confidence level for reading the given file as a DICOM segmentation. */
    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    /**
     * \brief Write the MultiLabelSegmentation as DICOM Segmentation files, one per group.
     *
     * Group 0 is written to the output location; the file of group g inserts "_<g>" before the
     * last dot of the file name, or appends it if the file name has none or consists of digits and
     * dots only, such as a UID. The writer adds no extension of its own. Group names are not kept:
     * DICOM SEG has no field meant for them.
     * Native NRRD and the segmentation stack keep them.
     *
     * The segments of a group are numbered 1..N in ascending label value order. dcmqi writes no
     * segment for a label without voxels: such a label is lost, and only the labels with voxels
     * count. These keep their values if they are 1..N; otherwise each gets its rank among them. In
     * the labelmap encoding with values 1..N, a label without voxels below one with voxels makes
     * dcmqi fail.
     *
     * The Segment Label is the label's name; for a label without a name it is the description, and
     * without either the meaning of the Segmented Property Type written. A Segment Label or Segment
     * Algorithm Name longer than the 64 characters of VR LO is cut and marked with "[...]", with a
     * warning. BodyPartExamined is taken only from the segmentation's property DICOM.0018.0015,
     * which a segmentation inherits from the image it is created from, and left out without it;
     * SeriesDescription is taken from DICOM.0008.103E.
     *
     * \throw DICOMSegStrictModeException in strict mode if the segmentation lacks a contract item
     * (DICOMSegmentationPropertyHelper::Validate) or a source-image relation with per-slice UIDs;
     * a retry in synthetic mode can succeed.
     * \throw DICOMSegUnrepresentableValueException naming the label for a label value above 32767,
     * a Segment Label or, for an AUTOMATIC or SEMIAUTOMATIC label, an algorithm name that is no
     * valid LO value without Specific Character Set (a backslash, non-ASCII or control characters),
     * a description or Tracking ID that is no valid ST or UT value without it (non-ASCII characters,
     * or control characters other than TAB, LF, FF, CR and ESC), or a Tracking UID that is no valid
     * UID; only changing the label lets the write succeed.
     * \throw mitk::Exception naming the relation for a source-image relation with per-slice UIDs
     * whose source series UID is missing or no valid UID, or one of whose slices has a SOP Instance
     * UID that is no valid UID or lacks a valid SOP Class UID (slices without a SOP Instance UID are
     * skipped); naming the attribute for a patient, study or frame of reference value that is no
     * valid value of its VR without Specific Character Set (non-ASCII characters, a value outside
     * the syntax of the VR, such as an invalid date or UID, or a UI, CS or TM value over its maximum
     * length) and for an empty Frame of Reference UID; naming the option for an option value outside
     * its choices; and if synthetic mode cannot fill a contract item, if no source-image reference
     * lies on a slice of the segmentation, or if dcmqi fails. A failure of ITK, of dcmqi or of saving
     * in a later group leaves the files of the earlier groups.
     *
     * The labels, the UIDs of the relations and the patient, study and frame of reference values are
     * checked first: before the strict-mode refusal, before synthetic mode changes the segmentation,
     * and before any file is written.
     */
    void Write() override;

    /** \brief Return the confidence level for writing the given data as a DICOM segmentation. */
    ConfidenceLevel GetWriterConfidenceLevel() const override;

  protected:
    /**
     * \brief Read a BINARY or LABELMAP DICOM segmentation.
     *
     * A BINARY segmentation yields one group if SegmentsOverlap is NO and one group
     * per segment otherwise; a LABELMAP yields one group. The format has no
     * opacity, visibility or lock state, so every label gets the label defaults
     * and is visible.
     *
     * \return A vector with the one mitk::MultiLabelSegmentation read.
     * \throw mitk::Exception naming the file if it cannot be read: it is no DICOM
     * file, its SOP class is neither Segmentation Storage nor Label Map
     * Segmentation Storage, it is a FRACTIONAL segmentation, it is a LABELMAP
     * whose pixel values and Segment Sequence disagree, or dcmqi fails to convert it.
     */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    DICOMSegmentationIO *IOClone() const override;

    // -------------- DICOMSegmentationIO specific functions -------------

    /**
     * \brief Populate a DCMQI metadata handler in memory for one group.
     *
     * Out-parameter (rather than return-by-value) because
     * dcmqi::JSONSegmentationMetaInformationHandler owns raw
     * SegmentAttributes pointers and its destructor deletes them. A copy
     * or move (which the compiler is free to materialise on return) would
     * double-free those pointers; constructing the handler at the call
     * site and populating it in place removes the question.
     *
     * \pre input must be a valid (non-null) MultiLabelSegmentation pointer.
     * \pre layer must be a valid group index of input.
     */
    void BuildMetaInfoHandler(const MultiLabelSegmentation *input,
                              int layer,
                              dcmqi::JSONSegmentationMetaInformationHandler &handler) const;
    void SetLabelProperties(Label *label, dcmqi::SegmentAttributes *segmentAttribute);

    /**
     * \brief Build a MultiLabelSegmentation from a Sup 243 labelmap SEG.
     *
     * A labelmap SEG carries every segment in a single image whose pixel
     * values are the segment numbers, so the result is exactly one MITK
     * group. Labels are driven from the SEG's Segment Sequence (metaInfo) so
     * the metadata is authoritative for what labels exist; mismatch between
     * pixel grid and metadata in either direction throws rather than
     * silently dropping content.
     *
     * \pre converter.dcmSegmentation2itkimage() must have been called
     *      successfully.
     * \pre converter.isLabelmap() must be true.
     * \pre metaInfo.read() must have been called.
     */
    MultiLabelSegmentation::Pointer ReadLabelmapSegmentation(
      dcmqi::Dicom2ItkConverterBase &converter,
      dcmqi::JSONSegmentationMetaInformationHandler &metaInfo);

    /**
     * \brief Build a MultiLabelSegmentation from a binary DICOM SEG.
     *
     * A binary SEG arrives as one image per segment. When
     * assumeOverlappingSegments is true each segment is placed in its own
     * MITK group because segment images may overlap; otherwise all segments
     * share a single group.
     *
     * \pre converter.dcmSegmentation2itkimage() must have been called
     *      successfully.
     * \pre converter.isLabelmap() must be false.
     * \pre metaInfo.read() must have been called.
     */
    MultiLabelSegmentation::Pointer ReadBinarySegmentation(
      dcmqi::Dicom2ItkConverterBase &converter,
      dcmqi::JSONSegmentationMetaInformationHandler &metaInfo,
      bool assumeOverlappingSegments);
  };
} // end of namespace mitk

#endif
