/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCESTGenericDICOMReaderService_h
#define mitkCESTGenericDICOMReaderService_h

#include <mitkBaseDICOMReaderService.h>

namespace mitk {

  /**
   * \brief Service wrapper for generic CEST DICOM loading with manual meta data configuration.
   *
   * Auto-selects the best DICOMFileReader from the DICOM module using
   * mitk::DICOMFileReaderSelector and loads CEST-relevant meta data from a
   * provided cest_meta.json file or from user-supplied reader options.
   */
  class CESTDICOMManualReaderService : public BaseDICOMReaderService
  {
  public:
    /**
     * \brief Construct with a specific MIME type and description.
     *
     * \param mimeType The custom MIME type this reader handles.
     * \param description Human-readable description of this reader service.
     */
    CESTDICOMManualReaderService(const CustomMimeType& mimeType, const std::string& description);

    /**
     * \brief Read DICOM files and enrich the result with CEST meta data.
     *
     * Uses the AbstractFileReader Read function and adds extra steps
     * for parsing CEST meta data from JSON files or reader options.
     *
     * \return A vector of loaded BaseData objects with CEST properties.
     */
    using AbstractFileReader::Read;
    std::vector<itk::SmartPointer<BaseData> > Read() override;

    /** \brief Return the available reader options including CEST-specific parameters. */
    Options GetOptions() const override;

    /**
     * \brief Return the value of a specific reader option.
     * \param name The option name to query.
     * \return The option value.
     */
    us::Any GetOption(const std::string& name) const override;

  protected:
    CESTDICOMManualReaderService(const CESTDICOMManualReaderService&) = default;
    CESTDICOMManualReaderService& operator=(const CESTDICOMManualReaderService&) = delete;

    /** \brief Return the path to the CEST meta JSON file in the same directory as the DICOM files. */
    std::string GetCESTMetaFilePath() const;

    /** \brief Return the path to the TREC file in the same directory as the DICOM files. */
    std::string GetTRECFilePath() const;

    /** \brief Return the path to the LIST file in the same directory as the DICOM files. */
    std::string GetLISTFilePath() const;

    /**
     * \brief Return the reader instance that should be used for loading.
     *
     * \param relevantFiles List of file paths to consider for reader selection.
     * \return A configured DICOMFileReader instance.
     */
    mitk::DICOMFileReader::Pointer GetReader(const mitk::StringList& relevantFiles) const override;

  private:
    CESTDICOMManualReaderService* Clone() const override;
  };

  /** \brief Return the DICOMTagPath for the Imaging Frequency DICOM tag. */
  DICOMTagPath DICOM_IMAGING_FREQUENCY_PATH();
}

#endif
