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

// MITK
#include <mitkMessage.h>


namespace mitk {
  class BaseData;
  class DataStorage;
}

namespace itk {
  template<class T> class SmartPointer;
}

namespace mitk {
  /**
  * \brief The common interface for all MITK file readers.
  *
  * Implementations of this interface must be registered as a service
  * to make themselve available via the service registry. If the
  * implementation is state-full, the service should be registered using
  * a PrototypeServiceFactory.
  *
  * The file reader implementation is associated with a mime-type, specified
  * in the service property PROP_MIMETYPE(). The specified mime-type should
  * have a corresponding IMimeType service object, registered by the reader
  * or some other party.
  *
  * It is recommended to derive new implementations from AbstractFileReader,
  * which provides correct service registration semantics.
  *
  * \sa AbstractFileReader
  * \sa IMimeType
  * \sa FileReaderRegistry
  */
  struct MITK_CORE_EXPORT IFileReader
  {
    virtual ~IFileReader();

    typedef std::pair<std::string, bool> Option;
    typedef std::vector<std::string> OptionNames;
    typedef std::vector<Option> OptionList;

    typedef mitk::MessageAbstractDelegate1<float> ProgressCallback;

    /**
     * \brief Reads the specified file and returns its contents.
     */
    virtual std::vector<itk::SmartPointer<BaseData> > Read(const std::string& path) = 0;

    /**
     * \brief Reads the specified input stream and returns its contents.
     */
    virtual std::vector<itk::SmartPointer<BaseData> > Read(std::istream& stream) = 0;

    /**
    * \brief Reads the specified file and returns its contents.
    *
    * When reading a given file, multiple BaseData instances might be produces.
    * If a DataStorage instance is passed and the reader added a BaseData instance
    * to it, the second element in the returned pair will be set true.
    */
    virtual std::vector<std::pair<itk::SmartPointer<BaseData>,bool> > Read(const std::string& path, mitk::DataStorage& ds) = 0;

    /**
    * \brief Reads the specified input stream and returns its contents.
    */
    virtual std::vector<std::pair<itk::SmartPointer<BaseData>,bool> > Read(std::istream& stream, mitk::DataStorage& ds) = 0;

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
    virtual OptionList GetOptions() const = 0;

    /**
    * \brief Sets the options for this reader
    *
    * The best way to use this method is to retireve the options via GetOptions, manipulate the bool values,
    * and then set the options again.
    */
    virtual void SetOptions(const OptionList& options) = 0;

    /**
    * \brief Returns true if this writer can confirm that it can read this file and false otherwise.
    */
    virtual bool CanRead(const std::string& path) const = 0;

    /**
     * \brief Returns true if this writer can read from the specified stream.
     *
     * @param stream The input stream.
     * @return \c true if the stream can be read, \c false otherwise.
     */
    virtual bool CanRead(std::istream& stream) const = 0;

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

// This is the microservice declaration. Do not meddle!
US_DECLARE_SERVICE_INTERFACE(mitk::IFileReader, "org.mitk.IFileReader")

#endif /* IFileReader_H_HEADER_INCLUDED_C1E7E521 */
