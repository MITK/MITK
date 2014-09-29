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

#ifndef IFileReader_H_HEADER_INCLUDED_C1E7E521
#define IFileReader_H_HEADER_INCLUDED_C1E7E521

// Macro
#include <MitkCoreExports.h>

// Microservices
#include <usServiceInterface.h>
#include <usAny.h>

// MITK
#include <mitkMessage.h>
#include <mitkDataStorage.h>


namespace mitk {
  class BaseData;
}

namespace itk {
  template<class T> class SmartPointer;
}

namespace mitk {

  /**
   * \brief The common interface for all MITK file readers.
   * \ingroup IO
   *
   * Implementations of this interface must be registered as a service
   * to make themselve available via the service registry. If the
   * implementation is state-full, the service should be registered using
   * a PrototypeServiceFactory.
   *
   * The file reader implementation is associated with a mime-type, specified
   * in the service property PROP_MIMETYPE(). The specified mime-type should
   * have a corresponding CustomMimeType service object, registered by the reader
   * or some other party.
   *
   * It is recommended to derive new implementations from AbstractFileReader or
   * from AbstractFileIO (if both reader and writer is implemented),
   * which provide correct service registration semantics.
   *
   * \sa AbstractFileReader
   * \sa AbstractFileIO
   * \sa CustomMimeType
   * \sa FileReaderRegistry
   * \sa IFileWriter
   */
  struct MITK_CORE_EXPORT IFileReader
  {

    // The order of the enum values is important: it is used
    // to rank writer implementations
    enum ConfidenceLevel
    {
      Unsupported = 0,
      PartiallySupported = 8,
      Supported = 16
    };

    virtual ~IFileReader();

    typedef std::map<std::string, us::Any> Options;

    typedef mitk::MessageAbstractDelegate1<float> ProgressCallback;

    /**
     * \brief Set the input file name.
     * \param location The file name to read from.
     */
    virtual void SetInput(const std::string& location) = 0;

    virtual void SetInput(const std::string &location, std::istream* is) = 0;

    virtual std::string GetInputLocation() const = 0;

    virtual std::istream* GetInputStream() const = 0;

    /**
     * \brief Reads the specified file or input stream and returns its contents.
     *
     * \return A list of created BaseData objects.
     * \throws mitk::Exception
     */
    virtual std::vector<itk::SmartPointer<BaseData> > Read() = 0;

    /**
     * \brief Reads the specified file or input stream, loading its
     * contents into the provided DataStorage.
     *
     * \param ds The DataStorage to which the data is added.
     * \return The set of added DataNodes to \c ds.
     * \throws mitk::Exception
     */
    virtual DataStorage::SetOfObjects::Pointer Read(mitk::DataStorage& ds) = 0;

    virtual ConfidenceLevel GetConfidenceLevel() const = 0;

    /**
     * \brief returns a list of the supported Options
     *
     * Initially, the reader contains a set of standard options. These can be retrieved,
     * manipulated and set again. To activate or deactivate an option, just set it's bool
     * value accordingly. All supported options are in the default set, it does not
     * make sense to add strings artificially after retrieving a reader - the specific
     * implementation decides which options it wants to support.
     * If no options are supported, an empty list is returned.
     */
    virtual Options GetOptions() const = 0;

    virtual us::Any GetOption(const std::string& name) const = 0;

    /**
     * \brief Sets the options for this reader
     *
     * The best way to use this method is to retireve the options via GetOptions, manipulate the bool values,
     * and then set the options again.
     */
    virtual void SetOptions(const Options& options) = 0;

    virtual void SetOption(const std::string& name, const us::Any& value) = 0;

    virtual void AddProgressCallback(const ProgressCallback& callback) = 0;

    virtual void RemoveProgressCallback(const ProgressCallback& callback) = 0;

    /**
     * @brief Service property name for a description.
     *
     * The property value must be of type \c std::string.
     *
     * @return The property name.
     */
    static std::string PROP_DESCRIPTION();

    /**
     * @brief Service property name for the mime-type associated with this file reader.
     *
     * The property value must be of type \c std::string.
     *
     * @return The property name.
     */
    static std::string PROP_MIMETYPE();

  };

} // namespace mitk

US_DECLARE_SERVICE_INTERFACE(mitk::IFileReader, "org.mitk.IFileReader")

#endif /* IFileReader_H_HEADER_INCLUDED_C1E7E521 */
