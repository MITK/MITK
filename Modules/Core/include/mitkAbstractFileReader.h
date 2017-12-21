/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef AbstractFileReader_H_HEADER_INCLUDED_C1E7E521
#define AbstractFileReader_H_HEADER_INCLUDED_C1E7E521

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
   * @brief Base class for creating mitk::BaseData objects from files or streams.
   * @ingroup IO
   */
  class MITKCORE_EXPORT AbstractFileReader : public mitk::IFileReader
  {
  public:
    void SetInput(const std::string &location) override;

    void SetInput(const std::string &location, std::istream *is) override;

    std::string GetInputLocation() const override;

    std::istream *GetInputStream() const override;

    MimeType GetRegisteredMimeType() const;

    /**
     * @brief Reads a path or stream and creates a list of BaseData objects.
     *
     * This method must be implemented for each specific reader. Call
     * GetInputStream() first and check for a non-null stream to read from.
     * If the input stream is \c nullptr, use GetInputLocation() to read from a local
     * file-system path.
     *
     * If the reader cannot use streams directly, use GetLocalFileName() instead.
     *
     * @return The created BaseData objects.
     * @throws mitk::Exception
     *
     * @see GetLocalFileName()
     * @see IFileReader::Read()
     */
    std::vector<itk::SmartPointer<BaseData>> Read() override = 0;

    DataStorage::SetOfObjects::Pointer Read(mitk::DataStorage &ds) override;

    ConfidenceLevel GetConfidenceLevel() const override;

    Options GetOptions() const override;
    us::Any GetOption(const std::string &name) const override;

    void SetOptions(const Options &options) override;
    void SetOption(const std::string &name, const us::Any &value) override;

    void AddProgressCallback(const ProgressCallback &callback) override;

    void RemoveProgressCallback(const ProgressCallback &callback) override;

    /**
     * Associate this reader with the MIME type returned by the current IMimeTypeProvider
     * service for the provided extension if the MIME type exists, otherwise registers
     * a new MIME type when RegisterService() is called.
     *
     * If no MIME type for \c extension is already registered, a call to RegisterService()
     * will register a new MIME type and associate this reader instance with it. The MIME
     * type id can be set via SetMimeType() or it will be auto-generated using \c extension,
     * having the form "application/vnd.mitk.<extension>".
     *
     * @param extension The file extension (without a leading period) for which a registered
     *        mime-type object is looked up and associated with this reader instance.
     * @param description A human readable description of this reader.
     */
    us::ServiceRegistration<IFileReader> RegisterService(us::ModuleContext *context = us::GetModuleContext());
    void UnregisterService();

    /**
     * @return A list of files that were loaded during the last call of Read. Has to be filled by the actual reader class.
     */
    std::vector< std::string > GetReadFiles() override;

  protected:
    /**
     * @brief An input stream wrapper.
     *
     * If a reader can only work with input streams, use an instance
     * of this class to either wrap the specified input stream or
     * create a new input stream based on the input location in the
     * file system.
     */
    class MITKCORE_EXPORT InputStream : public std::istream
    {
    public:
      InputStream(IFileReader *writer, std::ios_base::openmode mode = std::ios_base::in);
      ~InputStream() override;

    private:
      std::istream *m_Stream;
    };

    AbstractFileReader();
    ~AbstractFileReader() override;

    AbstractFileReader(const AbstractFileReader &other);

    /**
     * Associate this reader instance with the given MIME type.
     *
     * If \c mimeType does not provide an extension list, an already
     * registered mime-type object is used. Otherwise, the first entry in
     * the extensions list is used to construct a mime-type name and
     * register it as a new CustomMimeType service object in the default
     * implementation of RegisterMimeType().
     *
     * @param mimeType The mime type this reader can read.
     * @param description A human readable description of this reader.
     *
     * @throws std::invalid_argument if \c mimeType is empty.
     *
     * @see RegisterService
     */
    explicit AbstractFileReader(const CustomMimeType &mimeType, const std::string &description);

    virtual us::ServiceProperties GetServiceProperties() const;

    /**
     * Registers a new CustomMimeType service object.
     *
     * This method is called from RegisterService and the default implementation
     * registers a new mime-type service object if all of the following conditions
     * are true:
     *
     *  - TODO
     *
     * @param context
     * @return
     * @throws std::invalid_argument if \c context is nullptr.
     */
    virtual us::ServiceRegistration<CustomMimeType> RegisterMimeType(us::ModuleContext *context);

    void SetMimeType(const CustomMimeType &mimeType);

    /**
     * @return The mime-type this reader can handle.
     */
    const CustomMimeType *GetMimeType() const;

    void SetMimeTypePrefix(const std::string &prefix);
    std::string GetMimeTypePrefix() const;

    void SetDescription(const std::string &description);
    std::string GetDescription() const;

    void SetDefaultOptions(const Options &defaultOptions);
    Options GetDefaultOptions() const;

    /**
     * \brief Set the service ranking for this file reader.
     *
     * Default is zero and should only be chosen differently for a reason.
     * The ranking is used to determine which reader to use if several
     * equivalent readers have been found.
     * It may be used to replace a default reader from MITK in your own project.
     * E.g. if you want to use your own reader for nrrd files instead of the default,
     * implement it and give it a higher ranking than zero.
     */
    void SetRanking(int ranking);
    int GetRanking() const;

    /**
     * @brief Get a local file name for reading.
     *
     * This is a convenience method for readers which cannot work natively
     * with input streams. If no input stream has been been set,
     * this method just returns the result of GetLocation(). However, if
     * SetLocation(std::string, std::istream*) has been called with a non-null
     * input stream, this method writes the contents of the stream to a temporary
     * file and returns the name of the temporary file.
     *
     * The temporary file is deleted when either SetLocation(std::string, std::istream*)
     * is called again with a different input stream or the destructor of this
     * class is called.
     *
     * This method does not validate file names set via SetInput(std::string).
     *
     * @return A file path in the local file-system for reading.
     */
    std::string GetLocalFileName() const;

    virtual void SetDefaultDataNodeProperties(DataNode *node, const std::string &filePath);

    std::vector< std::string > m_ReadFiles;

  private:
    AbstractFileReader &operator=(const AbstractFileReader &other);

    virtual mitk::IFileReader *Clone() const = 0;

    class Impl;
    std::unique_ptr<Impl> d;
  };

} // namespace mitk

#endif /* AbstractFileReader_H_HEADER_INCLUDED_C1E7E521 */
