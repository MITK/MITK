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

#include <mitkIFileIO.h>
#include <mitkDataStorage.h>

#include <mitkServiceInterface.h>

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
struct MITK_CORE_EXPORT IFileReader : public IFileIO
{

  virtual ~IFileReader();

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

};

} // namespace mitk

MITK_DECLARE_SERVICE_INTERFACE(mitk::IFileReader, "org.mitk.IFileReader")

#endif /* IFileReader_H_HEADER_INCLUDED_C1E7E521 */
