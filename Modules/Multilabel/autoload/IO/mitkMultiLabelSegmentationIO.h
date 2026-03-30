/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiLabelSegmentationIO_h
#define mitkMultiLabelSegmentationIO_h

#include <mitkAbstractFileIO.h>
#include <mitkLabelSetImage.h>

namespace mitk
{
  /**
   * \brief Read and write MultiLabelSegmentation objects to/from NRRD files.
   *
   * mitk::Identifiable UID is supported and will be serialized.
   *
   * \ingroup Process
   */
  class MultiLabelSegmentationIO : public mitk::AbstractFileIO
  {
  public:
    typedef mitk::MultiLabelSegmentation InputType;

    /** \brief Default constructor. Registers reader/writer for the MultiLabelSegmentation MIME type. */
    MultiLabelSegmentationIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    /** \brief Return the confidence level for reading the given file as a MultiLabelSegmentation. */
    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    /** \brief Write the MultiLabelSegmentation to an NRRD file with embedded label metadata. */
    void Write() override;

    /** \brief Return the confidence level for writing the given data. */
    ConfidenceLevel GetWriterConfidenceLevel() const override;

  protected:
    /**
     * \brief Read MultiLabelSegmentation data from the file system.
     * \return A vector of mitk::LabelSetImages.
     * \throw mitk::Exception if an error occurs during NRRD header parsing.
     */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

    // Fills the m_DefaultMetaDataKeys vector with default values
    virtual void InitializeDefaultMetaDataKeys();

  private:
    MultiLabelSegmentationIO *IOClone() const override;

    std::vector<std::string> m_DefaultMetaDataKeys;
  };
} // end of namespace mitk

#endif
