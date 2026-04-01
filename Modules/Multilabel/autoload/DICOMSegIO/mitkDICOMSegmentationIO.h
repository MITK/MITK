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

#include <dcmqi/JSONSegmentationMetaInformationHandler.h>

#include <memory>

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
     * \brief Read DICOM segmentation objects from the file system.
     * \return A vector of mitk::LabelSetImages.
     * \throw mitk::Exception if an error occurs during reading.
     */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    DICOMSegmentationIO *IOClone() const override;

    // -------------- DICOMSegmentationIO specific functions -------------
    const std::string CreateMetaDataJsonFile(int layer);
    void SetLabelProperties(Label *label, dcmqi::SegmentAttributes *segmentAttribute);
  };
} // end of namespace mitk

#endif
