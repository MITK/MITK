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

    /** \brief Write the MultiLabelSegmentation as a DICOM Segmentation file. */
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
