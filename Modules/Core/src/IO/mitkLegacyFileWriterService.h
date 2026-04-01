/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLegacyFileWriterService_h
#define mitkLegacyFileWriterService_h

#include <mitkAbstractFileWriter.h>
#include <mitkFileWriter.h>

namespace mitk
{
  /**
   * \brief Adapter that wraps legacy mitk::FileWriter instances as micro service writers.
   *
   * This class bridges the old ITK object factory-based I/O system to the
   * CppMicroServices-based file writer framework. It wraps a legacy FileWriter
   * and delegates writing to it.
   *
   * \sa AbstractFileWriter
   * \sa LegacyFileReaderService
   * \sa FileWriter
   */
  class LegacyFileWriterService : public mitk::AbstractFileWriter
  {
  public:
    /**
     * \brief Construct and register a legacy writer service.
     *
     * Creates a MIME type from the file extensions supported by the legacy
     * writer, then registers this writer as a CppMicroServices service.
     *
     * \param[in] legacyWriter The legacy FileWriter to wrap.
     * \param[in] description A human-readable description for the writer service.
     */
    LegacyFileWriterService(mitk::FileWriter::Pointer legacyWriter, const std::string &description);

    ~LegacyFileWriterService() override;

    using AbstractFileWriter::Write;

    /**
     * \brief Write the input data using the wrapped legacy writer.
     *
     * Validates the output location, obtains a local file path, and delegates
     * writing to the wrapped legacy FileWriter.
     *
     * \throw mitk::Exception if the legacy writer was not properly initialized.
     */
    void Write() override;

    /**
     * \brief Determine the confidence level for writing the current input.
     *
     * Checks whether the wrapped legacy writer can handle the input data type.
     *
     * \return Supported if the legacy writer accepts the data, Unsupported otherwise.
     */
    ConfidenceLevel GetConfidenceLevel() const override;

  private:
    LegacyFileWriterService *Clone() const override;

    mitk::FileWriter::Pointer m_LegacyWriter;

    us::ServiceRegistration<IFileWriter> m_ServiceRegistration;
  };

} // namespace mitk

#endif
