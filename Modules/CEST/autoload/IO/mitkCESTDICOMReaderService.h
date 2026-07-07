/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCESTDICOMReaderService_h
#define mitkCESTDICOMReaderService_h

#include <mitkBaseDICOMReaderService.h>

namespace mitk {

  /**
   * \brief Service wrapper that auto-selects the best DICOMFileReader for CEST data.
   *
   * Uses mitk::DICOMFileReaderSelector to choose the best DICOMFileReader from
   * the DICOM module and loads additional CEST-specific meta data.
   */
  class CESTDICOMReaderService : public BaseDICOMReaderService
  {
  public:
    /** \brief Default constructor. Registers the reader with the CEST DICOM MIME type. */
    CESTDICOMReaderService();

    /**
     * \brief Construct with a custom description string.
     * \param description Human-readable description of this reader service.
     */
    CESTDICOMReaderService(const std::string& description);

    /**
     * \brief Read DICOM files and enrich the result with CEST meta data.
     *
     * Uses the BaseDICOMReaderService Read function and adds extra steps
     * for parsing and attaching CEST-specific meta data properties.
     *
     * \return A vector of loaded BaseData objects with CEST properties.
     */
    using AbstractFileReader::Read;
    std::vector<itk::SmartPointer<BaseData> > Read() override;

  protected:
    /**
     * \brief Return the reader instance that should be used for loading.
     *
     * The decision may be based on the passed list of relevant files.
     *
     * \param relevantFiles List of file paths to consider for reader selection.
     * \return A configured DICOMFileReader instance.
     */
    mitk::DICOMFileReader::Pointer GetReader(const mitk::StringList& relevantFiles) const override;

  private:

    CESTDICOMReaderService* Clone() const override;
  };

}

#endif
