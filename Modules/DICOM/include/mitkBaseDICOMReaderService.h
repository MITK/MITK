/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBaseDICOMReaderService_h
#define mitkBaseDICOMReaderService_h

#include <mitkAbstractFileReader.h>
#include <mitkDICOMFileReader.h>

#include <MitkDICOMExports.h>

namespace mitk {

  /**
   * \ingroup DICOMModule
   * \brief Base class for service wrappers that make DICOMFileReader from the DICOM module usable.
   *
   * This abstract class bridges the gap between the MITK DICOM file reading infrastructure
   * (DICOMFileReader) and the I/O service framework (AbstractFileReader). Subclasses must
   * implement GetReader() to provide a concrete DICOMFileReader for loading.
   *
   * \sa DICOMFileReader, DICOMFileReaderSelector, AbstractFileReader
   */
class MITKDICOM_EXPORT BaseDICOMReaderService : public AbstractFileReader
{
public:
  using AbstractFileReader::Read;

  /**
   * \brief Determine the confidence level for reading a given file.
   * \return The confidence level indicating how well this reader can handle the file.
   */
  IFileReader::ConfidenceLevel GetConfidenceLevel() const override;

protected:
  /**
   * \brief Construct with the default DICOM MIME type and a description string.
   * \param[in] description Human-readable description of this reader service.
   */
  BaseDICOMReaderService(const std::string& description);

  /**
   * \brief Construct with a custom MIME type and a description string.
   * \param[in] customType The custom MIME type to register for.
   * \param[in] description Human-readable description of this reader service.
   */
  BaseDICOMReaderService(const mitk::CustomMimeType& customType, const std::string& description);

  /**
   * \brief Perform the actual reading of DICOM data.
   *
   * Uses GetDICOMFilesInSameDirectory() and GetReader() to load image data
   * and puts it into BaseData instances.
   *
   * \return A vector of loaded BaseData smart pointers.
   */
  std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  /**
   * \brief Returns the list of all DICOM files in the same directory as this->GetLocalFileName().
   * \return A StringList of absolute file paths.
   */
  mitk::StringList GetDICOMFilesInSameDirectory() const;

  /**
   * \brief Returns the reader instance that should be used for loading.
   *
   * The decision may be based on the passed list of relevant files.
   * Subclasses must implement this method.
   *
   * \param[in] relevantFiles The list of DICOM files to consider for reader selection.
   * \return A smart pointer to the DICOMFileReader to use.
   */
  virtual mitk::DICOMFileReader::Pointer GetReader(const mitk::StringList& relevantFiles) const = 0;

  /**
   * \brief Set whether only files from the same DICOM series should be loaded.
   * \param[in] onlyRegardOwnSeries If true, only files sharing the same Series Instance UID are loaded.
   */
  void SetOnlyRegardOwnSeries(bool onlyRegardOwnSeries);

  /**
   * \brief Get whether only files from the same DICOM series are loaded.
   * \return true if loading is restricted to the same series.
   */
  bool GetOnlyRegardOwnSeries() const;

private:
  /** Flags that controls if the read() operation should only regard DICOM files of the same series
  if the specified GetLocalFileName() is a file. If it is a directory, this flag has no impact (it is
  assumed false then).
  */
  bool m_OnlyRegardOwnSeries = true;
};


class IPropertyProvider;

/**
 * \brief Generate a human-readable name from DICOM properties of a data object.
 *
 * Helper function that generates a name string (e.g. for DataNode names) from the DICOM
 * properties of the passed provider instance.
 *
 * \param[in] provider The property provider to extract DICOM properties from.
 * \return A name string derived from DICOM tags. If the instance is nullptr or has no
 *         DICOM properties, DataNode::NO_NAME_VALUE() will be returned.
 * \sa IPropertyProvider
 */
std::string MITKDICOM_EXPORT GenerateNameFromDICOMProperties(const mitk::IPropertyProvider* provider);

}

#endif
