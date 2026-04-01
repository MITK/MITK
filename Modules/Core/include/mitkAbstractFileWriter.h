/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAbstractFileWriter_h
#define mitkAbstractFileWriter_h

// Macro
#include <MitkCoreExports.h>

// MITK
#include <mitkIFileWriter.h>
#include <mitkMimeType.h>

// Microservices
#include <usGetModuleContext.h>
#include <usServiceProperties.h>
#include <usServiceRegistration.h>

#include <memory>

namespace us
{
  struct PrototypeServiceFactory;
}

namespace mitk
{
  class CustomMimeType;

  /**
   * \ingroup IO
   *
   * \brief Base class for writing mitk::BaseData objects to files or streams.
   *
   * In general, all file writers should derive from this class to ensure that
   * the implementation is exposed to the CppMicroServices framework and is
   * automatically available throughout MITK. Subclasses need to implement
   * Write() and Clone().
   *
   * \sa IFileWriter
   * \sa AbstractFileIO
   * \sa AbstractFileReader
   * \sa FileWriterSelector
   */
  class MITKCORE_EXPORT AbstractFileWriter : public mitk::IFileWriter
  {
  public:
    /**
     * \brief Set the input data for writing.
     * \param[in] data The BaseData object to write.
     */
    void SetInput(const BaseData *data) override;

    /**
     * \brief Get the input data.
     * \return Pointer to the BaseData, or \c nullptr if none was set.
     */
    const BaseData *GetInput() const override;

    /**
     * \brief Set the output file-system path, clearing any output stream.
     * \param[in] location The file path to write to.
     */
    void SetOutputLocation(const std::string &location) override;

    /**
     * \brief Get the current output location.
     * \return The file path or label set via SetOutputLocation() or SetOutputStream().
     */
    std::string GetOutputLocation() const override;

    /**
     * \brief Set an output stream and associated location label.
     * \param[in] location A file-system path or custom label.
     * \param[in] os The output stream, or \c nullptr to clear.
     */
    void SetOutputStream(const std::string &location, std::ostream *os) override;

    /**
     * \brief Get the current output stream.
     * \return The output stream, or \c nullptr if writing to a file path.
     */
    std::ostream *GetOutputStream() const override;

    /**
     * \brief Write the base data to the specified location or output stream.
     *
     * Subclasses must implement this method. Call GetOutputStream() first and
     * check for a non-null stream to write to. If the output stream is \c nullptr,
     * use GetOutputLocation() to write to a local file-system path.
     *
     * If the writer cannot use streams directly, use LocalFile to retrieve
     * a temporary local file name instead.
     *
     * \throws mitk::Exception if writing fails.
     *
     * \sa LocalFile
     * \sa IFileWriter::Write()
     */
    void Write() override = 0;

    /**
     * \brief Determine confidence for writing the current input data.
     * \return Supported if the input data's class hierarchy matches the writer's
     *         BaseData type, Unsupported otherwise.
     */
    ConfidenceLevel GetConfidenceLevel() const override;

    /**
     * \brief Get the MIME type that was registered for this writer.
     * \return The registered MimeType, which may be invalid if not yet registered.
     */
    MimeType GetRegisteredMimeType() const;

    /**
     * \brief Get the current option values.
     * \return A map of option names to their current values.
     */
    Options GetOptions() const override;

    /**
     * \brief Get the value of a specific option.
     * \param[in] name The option name.
     * \return The option value.
     */
    us::Any GetOption(const std::string &name) const override;

    /**
     * \brief Set multiple options at once.
     * \param[in] options A map of option names to their new values.
     */
    void SetOptions(const Options &options) override;

    /**
     * \brief Set a single option value.
     * \param[in] name The option name.
     * \param[in] value The new option value.
     */
    void SetOption(const std::string &name, const us::Any &value) override;

    /**
     * \brief Register a progress callback.
     * \param[in] callback The callback delegate to add.
     */
    void AddProgressCallback(const ProgressCallback &callback) override;

    /**
     * \brief Remove a previously registered progress callback.
     * \param[in] callback The callback delegate to remove.
     */
    void RemoveProgressCallback(const ProgressCallback &callback) override;

    /**
     * \brief Register this writer as a CppMicroServices service.
     *
     * Registers this writer with its mime-type in the service registry using
     * a PrototypeServiceFactory, so that each consumer gets its own clone.
     *
     * \param[in] context The module context for service registration. Defaults
     *            to the current module context.
     * \return The service registration object, or an invalid registration if
     *         the writer could not be registered.
     */
    us::ServiceRegistration<IFileWriter> RegisterService(us::ModuleContext *context = us::GetModuleContext());

    /**
     * \brief Unregister this writer from the CppMicroServices registry.
     */
    void UnregisterService();

  protected:
    /**
     * \brief A local file representation for writers that cannot use streams.
     *
     * If a writer can only work with local files, construct an instance
     * of this class. It provides either the original output location (if
     * no output stream was set) or a temporary file name. On destruction,
     * the temporary file's contents are flushed to the output stream and
     * the temporary file is removed.
     */
    class MITKCORE_EXPORT LocalFile
    {
    public:
      /**
       * \brief Construct a LocalFile for the given writer.
       * \param[in] writer The IFileWriter whose output configuration to use.
       */
      LocalFile(IFileWriter *writer);

      /**
       * \brief Destructor. Writes temporary file contents to the output stream
       *        (if any) and removes the temporary file.
       */
      ~LocalFile();

      /**
       * \brief Get a local file name for writing.
       * \return The output location if no stream is set, or a path to a
       *         newly created temporary file otherwise.
       */
      std::string GetFileName();

    private:
      // disabled
      LocalFile();
      LocalFile(const LocalFile &);
      LocalFile &operator=(const LocalFile &other);

      struct Impl;
      std::unique_ptr<Impl> d;
    };

    /**
     * \brief An output stream wrapper for writer implementations.
     *
     * If a writer can only work with output streams, use an instance of this
     * class. It will either wrap the output stream already set on the writer,
     * or open a new file stream based on the writer's output location.
     *
     * \sa GetOutputStream()
     * \sa GetOutputLocation()
     */
    class MITKCORE_EXPORT OutputStream : public std::ostream
    {
    public:
      /**
       * \brief Construct an OutputStream from a writer's configured output.
       * \param[in] writer The IFileWriter whose output to wrap.
       * \param[in] mode The open mode for the underlying file stream, if applicable.
       */
      OutputStream(IFileWriter *writer, std::ios_base::openmode mode = std::ios_base::trunc | std::ios_base::out);

      ~OutputStream() override;

    private:
      std::ostream *m_Stream;
    };

    ~AbstractFileWriter() override;

    /**
     * \brief Copy constructor.
     * \param[in] other The writer to copy from.
     */
    AbstractFileWriter(const AbstractFileWriter &other);

    /**
     * \brief Construct a writer for the given BaseData type name.
     * \param[in] baseDataType The class name of the BaseData sub-class this writer supports.
     */
    AbstractFileWriter(const std::string &baseDataType);

    /**
     * \brief Construct a writer for the given BaseData type, MIME type, and description.
     * \param[in] baseDataType The class name of the BaseData sub-class this writer supports.
     * \param[in] mimeType The MIME type this writer produces.
     * \param[in] description A human-readable description of this writer.
     */
    AbstractFileWriter(const std::string &baseDataType, const CustomMimeType &mimeType, const std::string &description);

    /**
     * \brief Get the service properties for registration.
     * \return A map of service property names to values.
     */
    virtual us::ServiceProperties GetServiceProperties() const;

    /**
     * \brief Register a new CustomMimeType service object.
     *
     * Called internally from RegisterService(). The default implementation
     * registers a new mime-type service if the mime-type has not been
     * registered yet.
     *
     * \param[in] context The module context for registration.
     * \return The service registration for the CustomMimeType.
     * \throw std::invalid_argument if \c context is \c nullptr.
     */
    virtual us::ServiceRegistration<CustomMimeType> RegisterMimeType(us::ModuleContext *context);

    /**
     * \brief Set the MIME type this writer can handle.
     * \param[in] mimeType The CustomMimeType to associate with this writer.
     */
    void SetMimeType(const CustomMimeType &mimeType);

    /**
     * \brief Get the MIME type this writer can handle.
     * \return Pointer to the internal CustomMimeType, or \c nullptr.
     */
    const CustomMimeType *GetMimeType() const;

    /**
     * \brief Set the prefix used for auto-generated MIME type names.
     * \param[in] prefix The prefix string (e.g. "application/vnd.mitk").
     */
    void SetMimeTypePrefix(const std::string &prefix);

    /**
     * \brief Get the current MIME type name prefix.
     * \return The prefix string.
     */
    std::string GetMimeTypePrefix() const;

    /**
     * \brief Set a human-readable description of this writer.
     * \param[in] description The description string, used in file dialogs.
     */
    void SetDescription(const std::string &description);

    /**
     * \brief Get the human-readable description of this writer.
     * \return The description string.
     */
    std::string GetDescription() const;

    /**
     * \brief Set the default options for this writer.
     * \param[in] defaultOptions A map of option names to default values.
     */
    void SetDefaultOptions(const Options &defaultOptions);

    /**
     * \brief Get the default options for this writer.
     * \return A map of option names to default values.
     */
    Options GetDefaultOptions() const;

    /**
     * \brief Set the service ranking for this file writer.
     * \param[in] ranking The ranking value. Default is zero.
     *
     * The ranking is used to determine which writer to use if several
     * equivalent writers have been found. Use a higher ranking to override
     * a default writer from MITK in your own project.
     */
    void SetRanking(int ranking);

    /**
     * \brief Get the service ranking for this file writer.
     * \return The ranking value.
     */
    int GetRanking() const;

    /**
     * \brief Set the name of the mitk::BaseData sub-class this writer supports.
     * \param[in] baseDataType The class name, as returned by BaseData::GetNameOfClass().
     */
    void SetBaseDataType(const std::string &baseDataType);

    /**
     * \brief Get the name of the mitk::BaseData sub-class this writer supports.
     * \return The BaseData type name.
     */
    virtual std::string GetBaseDataType() const;

    /**
     * \brief Validate the output location or stream.
     * \throw mitk::Exception if neither an output location nor an output stream is set.
     */
    void ValidateOutputLocation() const;

  private:
    AbstractFileWriter &operator=(const AbstractFileWriter &other);

    virtual mitk::IFileWriter *Clone() const = 0;

    class Impl;
    std::unique_ptr<Impl> d;
  };

} // namespace mitk

#endif
