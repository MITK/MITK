/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAbstractFileReader_h
#define mitkAbstractFileReader_h

// Macro
#include <MitkCoreExports.h>

// MITK
#include <mitkBaseData.h>
#include <mitkIFileReader.h>
#include <mitkMimeType.h>

// Microservices
#include <usGetModuleContext.h>
#include <usServiceProperties.h>
#include <usServiceRegistration.h>

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
   * \brief Base class for creating mitk::BaseData objects from files or streams.
   *
   * AbstractFileReader provides the default implementation of the IFileReader
   * interface. Subclasses only need to implement DoRead() and Clone(). The
   * base class handles service registration via CppMicroServices, mime-type
   * management, option handling, and progress callbacks.
   *
   * \sa IFileReader
   * \sa AbstractFileIO
   * \sa AbstractFileWriter
   * \sa FileReaderSelector
   */
  class MITKCORE_EXPORT AbstractFileReader : public mitk::IFileReader
  {
  public:
    /**
     * \brief Set the input file path, clearing any previously set input stream.
     * \param[in] location The absolute file-system path to read from.
     */
    void SetInput(const std::string &location) override;

    /**
     * \brief Set an input stream and associated location label.
     * \param[in] location A file-system path or custom label.
     * \param[in] is The input stream, or \c nullptr to clear.
     */
    void SetInput(const std::string &location, std::istream *is) override;

    /**
     * \brief Get the current input location.
     * \return The file path or label set via SetInput().
     */
    std::string GetInputLocation() const override;

    /**
     * \brief Get the current input stream.
     * \return The input stream, or \c nullptr if reading from a file path.
     */
    std::istream *GetInputStream() const override;

    /**
     * \brief Get the mime-type that was registered for this reader.
     * \return The registered MimeType, which may be invalid if not yet registered.
     */
    MimeType GetRegisteredMimeType() const;

    /**
     * \brief Read data and create a list of BaseData objects.
     *
     * The default implementation calls DoRead() and then annotates each
     * resulting BaseData with meta-information about the reader, MIME type,
     * input location, and reader options.
     *
     * \return A vector of created BaseData objects.
     * \throws mitk::Exception if reading fails.
     */
    std::vector<itk::SmartPointer<BaseData>> Read() override;

    /**
     * \brief Read data and add it to the given DataStorage.
     * \param[in,out] ds The DataStorage to add nodes to.
     * \return The set of DataNode objects added to \c ds.
     * \throws mitk::Exception if reading fails.
     */
    DataStorage::SetOfObjects::Pointer Read(mitk::DataStorage &ds) override;

    /**
     * \brief Determine confidence for reading the current input.
     * \return Supported if the input stream is valid or the input file exists,
     *         Unsupported otherwise.
     */
    ConfidenceLevel GetConfidenceLevel() const override;

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
     * \brief Register this reader as a CppMicroServices service.
     *
     * Registers this reader with its mime-type in the service registry using
     * a PrototypeServiceFactory, so that each consumer gets its own clone.
     * If no MIME type for the configured extension is already registered,
     * a new MIME type is registered automatically.
     *
     * \param[in] context The module context for service registration. Defaults
     *            to the current module context.
     * \return The service registration object, or an invalid registration if
     *         the reader could not be registered.
     */
    us::ServiceRegistration<IFileReader> RegisterService(us::ModuleContext *context = us::GetModuleContext());

    /**
     * \brief Unregister this reader from the CppMicroServices registry.
     */
    void UnregisterService();

    /**
     * \brief Get the list of files consumed during the last Read() call.
     * \return A vector of file paths. Must be populated by the DoRead() implementation.
     */
    std::vector< std::string > GetReadFiles() override;

    /**
     * \brief Set base data properties as a source of meta data for reading.
     * \param[in] properties The PropertyList to use, or \c nullptr to clear.
     * \sa IFileReader::SetProperties()
     */
    void SetProperties(const PropertyList* properties) override;

  protected:
    /**
     * \brief An input stream wrapper for reader implementations.
     *
     * If a reader can only work with input streams, use an instance of this
     * class. It will either wrap the input stream already set on the reader,
     * or open a new file stream based on the reader's input location.
     *
     * \sa GetInputStream()
     * \sa GetInputLocation()
     */
    class MITKCORE_EXPORT InputStream : public std::istream
    {
    public:
      /**
       * \brief Construct an InputStream from a reader's configured input.
       * \param[in] writer The IFileReader whose input to wrap (despite the parameter
       *            name, this is the reader instance).
       * \param[in] mode The open mode for the underlying file stream, if applicable.
       */
      InputStream(IFileReader *writer, std::ios_base::openmode mode = std::ios_base::in);

      ~InputStream() override;

    private:
      std::istream *m_Stream;
    };

    /** \brief Default constructor. */
    AbstractFileReader();

    ~AbstractFileReader() override;

    /**
     * \brief Copy constructor.
     * \param[in] other The reader to copy from.
     */
    AbstractFileReader(const AbstractFileReader &other);

    /**
     * \brief Associate this reader instance with the given MIME type.
     *
     * If \c mimeType does not provide an extension list, an already
     * registered mime-type object is used. Otherwise, the first entry in
     * the extensions list is used to construct a mime-type name and
     * register it as a new CustomMimeType service object in the default
     * implementation of RegisterMimeType().
     *
     * \param[in] mimeType The mime-type this reader can read.
     * \param[in] description A human-readable description of this reader.
     *
     * \throw std::invalid_argument if \c mimeType is empty.
     *
     * \sa RegisterService()
     */
    explicit AbstractFileReader(const CustomMimeType &mimeType, const std::string &description);

    /**
     * \brief Perform the actual reading operation.
     *
     * Subclasses must implement this method. The implementation should call
     * GetInputStream() first and check for a non-null stream to read from.
     * If the input stream is \c nullptr, use GetInputLocation() to read from
     * a local file-system path.
     *
     * If the reader cannot use streams directly, use GetLocalFileName() instead.
     *
     * \return A vector of created BaseData objects.
     * \throws mitk::Exception if reading fails.
     *
     * \sa GetLocalFileName()
     * \sa Read()
     */
    virtual std::vector<itk::SmartPointer<BaseData>> DoRead() = 0;


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
     * \brief Set the MIME type this reader can handle.
     * \param[in] mimeType The CustomMimeType to associate with this reader.
     */
    void SetMimeType(const CustomMimeType &mimeType);

    /**
     * \brief Get the MIME type this reader can handle.
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
     * \brief Set the human-readable description of this reader.
     * \param[in] description The description string.
     */
    void SetDescription(const std::string &description);

    /**
     * \brief Get the human-readable description of this reader.
     * \return The description string.
     */
    std::string GetDescription() const;

    /**
     * \brief Set the default options for this reader.
     * \param[in] defaultOptions A map of option names to default values.
     */
    void SetDefaultOptions(const Options &defaultOptions);

    /**
     * \brief Get the default options for this reader.
     * \return A map of option names to default values.
     */
    Options GetDefaultOptions() const;

    /**
     * \brief Set the service ranking for this file reader.
     * \param[in] ranking The ranking value. Default is zero.
     *
     * The ranking is used to determine which reader to use if several
     * equivalent readers have been found. Use a higher ranking to override
     * a default reader from MITK in your own project.
     */
    void SetRanking(int ranking);

    /**
     * \brief Get the service ranking for this file reader.
     * \return The ranking value.
     */
    int GetRanking() const;

    /**
     * \brief Get a local file name for reading.
     *
     * This is a convenience method for readers which cannot work natively
     * with input streams. If no input stream has been set, this method
     * returns the result of GetInputLocation(). If an input stream was set
     * via SetInput(std::string, std::istream*), this method writes the
     * stream contents to a temporary file and returns its path.
     *
     * The temporary file is deleted when SetInput() is called again with
     * a different input stream or when this reader is destroyed.
     *
     * This method does not validate file names set via SetInput(std::string).
     *
     * \return A file path in the local file-system for reading.
     */
    std::string GetLocalFileName() const;

    /**
     * \brief Set default properties on a DataNode after reading.
     * \param[in,out] node The DataNode to configure.
     * \param[in] filePath The file path used for reading (used to derive name and path properties).
     */
    virtual void SetDefaultDataNodeProperties(DataNode *node, const std::string &filePath);

    /**
     * \brief Get the properties set via SetProperties().
     * \return The current PropertyList, or \c nullptr if none was set.
     */
    const PropertyList* GetProperties() const override;

    /** \brief List of files consumed during the last Read() call. Must be filled by DoRead(). */
    std::vector< std::string > m_ReadFiles;

  private:
    AbstractFileReader &operator=(const AbstractFileReader &other);

    virtual mitk::IFileReader *Clone() const = 0;

    class Impl;
    std::unique_ptr<Impl> d;
  };

} // namespace mitk

#endif
