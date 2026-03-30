/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAbstractFileIO_h
#define mitkAbstractFileIO_h

#include <mitkAbstractFileReader.h>
#include <mitkAbstractFileWriter.h>

namespace mitk
{
#ifndef DOXYGEN_SKIP

  // Skip this code during Doxygen processing, because it only
  // exists to resolve name clashes when inheriting from both
  // AbstractFileReader and AbstractFileWriter.

  class AbstractFileIOReader : public AbstractFileReader
  {
  public:
    virtual ConfidenceLevel GetReaderConfidenceLevel() const { return AbstractFileReader::GetConfidenceLevel(); }
    ConfidenceLevel GetConfidenceLevel() const override { return this->GetReaderConfidenceLevel(); }
  protected:
    AbstractFileIOReader() {}
    AbstractFileIOReader(const CustomMimeType &mimeType, const std::string &description)
      : AbstractFileReader(mimeType, description)
    {
    }

  private:
    virtual IFileReader *ReaderClone() const = 0;
    IFileReader *Clone() const override { return ReaderClone(); }
  };

  struct AbstractFileIOWriter : public AbstractFileWriter
  {
    virtual ConfidenceLevel GetWriterConfidenceLevel() const { return AbstractFileWriter::GetConfidenceLevel(); }
    ConfidenceLevel GetConfidenceLevel() const override { return this->GetWriterConfidenceLevel(); }
  protected:
    AbstractFileIOWriter(const std::string &baseDataType) : AbstractFileWriter(baseDataType) {}
    AbstractFileIOWriter(const std::string &baseDataType,
                         const CustomMimeType &mimeType,
                         const std::string &description)
      : AbstractFileWriter(baseDataType, mimeType, description)
    {
    }

  private:
    virtual IFileWriter *WriterClone() const = 0;
    IFileWriter *Clone() const override { return WriterClone(); }
  };

#endif // DOXYGEN_SKIP

  /**
   * \ingroup IO
   *
   * \brief Abstract base class for implementing a combined file reader and writer.
   *
   * AbstractFileIO is a convenience class that inherits from both
   * AbstractFileReader and AbstractFileWriter (via intermediate adapters).
   * It shares a single MIME type between both reader and writer registrations
   * and provides unified option management. Subclasses must implement DoRead(),
   * Write(), and IOClone().
   *
   * \sa AbstractFileReader
   * \sa AbstractFileWriter
   * \sa IFileReader
   * \sa IFileWriter
   */
  class MITKCORE_EXPORT AbstractFileIO : public AbstractFileIOReader, public AbstractFileIOWriter
  {
  public:
    /**
     * \brief Get the reader-specific options.
     * \return A map of reader option names to their current values.
     */
    Options GetReaderOptions() const;

    /**
     * \brief Get the value of a specific reader option.
     * \param[in] name The option name.
     * \return The option value.
     */
    us::Any GetReaderOption(const std::string &name) const;

    /**
     * \brief Set multiple reader options at once.
     * \param[in] options A map of reader option names to values.
     */
    void SetReaderOptions(const Options &options);

    /**
     * \brief Set a single reader option.
     * \param[in] name The option name.
     * \param[in] value The new option value.
     */
    void SetReaderOption(const std::string &name, const us::Any &value);

    /**
     * \brief Get the writer-specific options.
     * \return A map of writer option names to their current values.
     */
    Options GetWriterOptions() const;

    /**
     * \brief Get the value of a specific writer option.
     * \param[in] name The option name.
     * \return The option value.
     */
    us::Any GetWriterOption(const std::string &name) const;

    /**
     * \brief Set multiple writer options at once.
     * \param[in] options A map of writer option names to values.
     */
    void SetWriterOptions(const Options &options);

    /**
     * \brief Set a single writer option.
     * \param[in] name The option name.
     * \param[in] value The new option value.
     */
    void SetWriterOption(const std::string &name, const us::Any &value);

    /**
     * \brief Get the reader confidence level for the current input.
     * \return The confidence level of the reader part.
     */
    ConfidenceLevel GetReaderConfidenceLevel() const override;

    /**
     * \brief Get the writer confidence level for the current input data.
     * \return The confidence level of the writer part.
     */
    ConfidenceLevel GetWriterConfidenceLevel() const override;

    /**
     * \brief Register both reader and writer as CppMicroServices services.
     *
     * The writer's MIME type is automatically synchronized with the reader's
     * MIME type if not explicitly set.
     *
     * \param[in] context The module context for service registration. Defaults
     *            to the current module context.
     * \return A pair of service registrations for the reader and writer.
     */
    std::pair<us::ServiceRegistration<IFileReader>, us::ServiceRegistration<IFileWriter>> RegisterService(
      us::ModuleContext *context = us::GetModuleContext());

  protected:
    /**
     * \brief Copy constructor.
     * \param[in] other The AbstractFileIO to copy from.
     */
    AbstractFileIO(const AbstractFileIO &other);

    /**
     * \brief Construct with a BaseData type name only.
     * \param[in] baseDataType The class name of the supported BaseData sub-class.
     */
    AbstractFileIO(const std::string &baseDataType);

    /**
     * \brief Construct with a BaseData type, MIME type, and description.
     *
     * If the given MIME type has nothing but its name set, the matching MIME type
     * is looked up in the service registry.
     *
     * \param[in] baseDataType The class name of the supported BaseData sub-class.
     * \param[in] mimeType The MIME type this reader/writer handles.
     * \param[in] description A human-readable description.
     *
     * \throw std::invalid_argument if \c mimeType is empty.
     *
     * \sa RegisterService()
     */
    explicit AbstractFileIO(const std::string &baseDataType,
                            const CustomMimeType &mimeType,
                            const std::string &description);

    /**
     * \brief Set the shared MIME type for both reader and writer.
     * \param[in] mimeType The CustomMimeType to set on both sides.
     */
    void SetMimeType(const CustomMimeType &mimeType);

    /**
     * \brief Get the shared MIME type.
     * \return Pointer to the reader's CustomMimeType. Logs a warning if the
     *         reader and writer MIME types differ.
     */
    const CustomMimeType *GetMimeType() const;

    /**
     * \brief Set the description for the reader part.
     * \param[in] description The reader description string.
     */
    void SetReaderDescription(const std::string &description);

    /**
     * \brief Get the description of the reader part.
     * \return The reader description string.
     */
    std::string GetReaderDescription() const;

    /**
     * \brief Set the description for the writer part.
     * \param[in] description The writer description string.
     */
    void SetWriterDescription(const std::string &description);

    /**
     * \brief Get the description of the writer part.
     * \return The writer description string.
     */
    std::string GetWriterDescription() const;

    /**
     * \brief Set default options for the reader part.
     * \param[in] defaultOptions A map of option names to default values.
     */
    void SetDefaultReaderOptions(const Options &defaultOptions);

    /**
     * \brief Get the default options for the reader part.
     * \return A map of option names to default values.
     */
    Options GetDefaultReaderOptions() const;

    /**
     * \brief Set default options for the writer part.
     * \param[in] defaultOptions A map of option names to default values.
     */
    void SetDefaultWriterOptions(const Options &defaultOptions);

    /**
     * \brief Get the default options for the writer part.
     * \return A map of option names to default values.
     */
    Options GetDefaultWriterOptions() const;

    /**
     * \brief Set the service ranking for the reader part.
     * \param[in] ranking The ranking value. Default is zero.
     *
     * Higher ranking values cause this reader to be preferred over
     * other readers for the same MIME type.
     */
    void SetReaderRanking(int ranking);

    /**
     * \brief Get the service ranking for the reader part.
     * \return The ranking value.
     */
    int GetReaderRanking() const;

    /**
     * \brief Set the service ranking for the writer part.
     * \param[in] ranking The ranking value. Default is zero.
     */
    void SetWriterRanking(int ranking);

    /**
     * \brief Get the service ranking for the writer part.
     * \return The ranking value.
     */
    int GetWriterRanking() const;

  private:
    AbstractFileIO &operator=(const AbstractFileIO &other);

    virtual AbstractFileIO *IOClone() const = 0;

    IFileReader *ReaderClone() const override;
    IFileWriter *WriterClone() const override;
  };
}

#endif
