/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLegacyFileReaderService_h
#define mitkLegacyFileReaderService_h

#include <mitkAbstractFileReader.h>

namespace mitk
{
  /**
   * \brief Adapter that wraps legacy mitk::FileReader instances as micro service readers.
   *
   * This class bridges the old ITK object factory-based I/O system ("mitkIOAdapter")
   * to the CppMicroServices-based file reader framework. It discovers legacy file
   * readers via the ITK object factory and delegates reading to them.
   *
   * \sa AbstractFileReader
   * \sa LegacyFileWriterService
   */
  class LegacyFileReaderService : public mitk::AbstractFileReader
  {
  public:
    /** \brief Copy constructor. */
    LegacyFileReaderService(const LegacyFileReaderService &other);

    /**
     * \brief Construct and register a legacy reader service.
     *
     * Creates a MIME type from the given file extensions and category, then
     * registers this reader as a CppMicroServices service.
     *
     * \param[in] extensions A list of supported file extensions (with or without leading dot).
     * \param[in] category The MIME type category (also used as the reader description).
     */
    LegacyFileReaderService(const std::vector<std::string> &extensions, const std::string &category);

    ~LegacyFileReaderService() override;

    using AbstractFileReader::Read;

  protected:
    /**
     * \brief Read data using legacy IOAdapter instances.
     *
     * Discovers all "mitkIOAdapter" instances via the ITK object factory, finds one
     * that can read the input file, and delegates to it.
     *
     * \return A vector of BaseData objects read from the file.
     * \throw mitk::Exception if no adapter can read the file.
     */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    LegacyFileReaderService *Clone() const override;

    us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };

} // namespace mitk

#endif
