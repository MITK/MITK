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

#include <mitkIFileIO.h>

#include <mitkServiceInterface.h>


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
 * The file writer implementation is associated with a mime-type, specified
 * in the service property PROP_MIMETYPE() and a mitk::BaseData sub-class
 * as specified in the PROP_BASEDATA_TYPE() service property.
 * The specified mime-type should have a corresponding CustomMimeType service
 * object, registered by the reader or some other party.
 *
 * It is recommended to derive new implementations from AbstractFileWriter or
 * AbstractFileIO (if both reader and writer is implemented),
 * which provide correct service registration semantics.
 *
 * \sa AbstractFileWriter
 * \sa AbstractFileIO
 * \sa CustomMimeType
 * \sa FileWriterRegistry
 * \sa IFileReader
 */
struct MITK_CORE_EXPORT IFileWriter : public IFileIO
{
  virtual ~IFileWriter();

  virtual void SetInput(const BaseData* data) = 0;
  virtual const BaseData* GetInput() const = 0;

  virtual void SetOutputLocation(const std::string& location) = 0;
  virtual std::string GetOutputLocation() const = 0;

  virtual void SetOutputStream(const std::string& location, std::ostream* os) = 0;
  virtual std::ostream* GetOutputStream() const = 0;

  virtual void Write() = 0;

  /**
   * @brief Service property name for the supported mitk::BaseData sub-class
   *
   * The property value must be of type \c std::string.
   *
   * @return The property name.
   */
  static std::string PROP_BASEDATA_TYPE();

};

} // namespace mitk

MITK_DECLARE_SERVICE_INTERFACE(mitk::IFileWriter, "org.mitk.IFileWriter")

#endif /* IFileWriter_H_HEADER_INCLUDED_C1E7E521 */
