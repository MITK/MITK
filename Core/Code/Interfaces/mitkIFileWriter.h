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

#ifndef IFileWriter_H_HEADER_INCLUDED_C1E7E521
#define IFileWriter_H_HEADER_INCLUDED_C1E7E521

// Macro
#include <MitkCoreExports.h>

// Microservices
#include <usServiceInterface.h>

// MITK
#include <mitkMessage.h>


namespace mitk {
  class BaseData;
}

namespace mitk {

  /**
   * \brief The common interface of all MITK file writers.
   *
   * Implementations of this interface must be registered as a service
   * to make themselve available via the service registry. If the
   * implementation is state-full, the service should be registered using
   * a PrototypeServiceFactory.
   *
   * It is recommended to derive new implementations from AbstractFileWriter,
   * which provides correct service registration semantics.
   *
   * \sa AbstractFileWriter
   * \sa FileWriterRegistry
   * \sa IFileReader
   */
  struct MITK_CORE_EXPORT IFileWriter
  {
    virtual ~IFileWriter();

    typedef std::pair<std::string, bool> FileServiceOption;
    typedef std::vector<FileServiceOption> OptionList;
    typedef std::vector<std::string> OptionNames;

    typedef mitk::MessageAbstractDelegate1<float> ProgressCallback;

    virtual void Write(const BaseData* data, const std::string& path ) = 0;

    virtual void Write(const BaseData* data, std::ostream& stream ) = 0;

    /**
     * \brief returns a list of the supported Options
     *
     * Options are strings that are treated as flags when passed to the write method.
     */
    virtual OptionList GetOptions() const = 0;

    virtual void SetOptions(const OptionList& options) = 0;

    /**
     * \brief Returns true if this reader can confirm that it can write \c data and false otherwise.
     */
    virtual bool CanWrite(const BaseData* data) const = 0;

    /**
     * \brief Returns a value between 0 and 1 depending on the progress of the writing process.
     * This method need not necessarily be implemented meaningfully, always returning zero is accepted.
     */
    virtual void AddProgressCallback(const ProgressCallback& callback) = 0;

    virtual void RemoveProgressCallback(const ProgressCallback& callback) = 0;

    // Microservice properties

    /**
     * @brief Service property name for the supported mitk::BaseData sub-class
     *
     * The property value must be of type \c std::string.
     *
     * @return The property name.
     */
    static std::string PROP_BASEDATA_TYPE();

    /**
     * @brief Service property name for a description.
     *
     * The property value must be of type \c std::string.
     *
     * @return The property name.
     */
    static std::string PROP_DESCRIPTION();

    /**
     * @brief Service property name for the mime-type associated with this file writer.
     *
     * The property value must be of type \c std::string.
     *
     * @return The property name.
     */
    static std::string PROP_MIMETYPE();

  };
} // namespace mitk

// This is the microservice declaration. Do not meddle!
US_DECLARE_SERVICE_INTERFACE(mitk::IFileWriter, "org.mitk.IFileWriter")

#endif /* IFileWriter_H_HEADER_INCLUDED_C1E7E521 */
