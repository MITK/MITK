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
 * \ingroup IO
 * \ingroup MicroServices_Interfaces
 *
 * \brief The common interface for all MITK file readers.
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
struct MITKCORE_EXPORT IFileReader : public IFileIO
{

  virtual ~IFileReader();

  /**
   * \brief Set the input location.
   * \param location The file name to read from.
   */
  virtual void SetInput(const std::string& location) = 0;

  /**
   * @brief Set an input stream to read from.
   * @param location A custom label for the input stream.
   * @param is The input stream.
   *
   * If \c is is \c NULL, this clears the current input stream and \c location
   * is interpreted as a file-system path. Otherwise, \c location is a custom
   * label describing the input stream \c is.
   */
  virtual void SetInput(const std::string &location, std::istream* is) = 0;

  /**
   * @brief Get the current input location.
   * @return The input location.
   */
  virtual std::string GetInputLocation() const = 0;

  /**
   * @brief Get the input stream.
   * @return The currently set input stream.
   */
  virtual std::istream* GetInputStream() const = 0;

  /**
   * \brief Reads the specified file or input stream and returns its contents.
   *
   * \return A list of created BaseData objects.
   *
   * If GetInputStream() returns a non-null value, this method must use
   * the returned stream object to read the data from. If no input stream
   * was set, the data must be read from the path returned by GetInputLocation().
   *
   * \throws mitk::Exception
   */
  virtual std::vector<itk::SmartPointer<BaseData> > Read() = 0;

  /**
   * \brief Reads the specified file or input stream, loading its
   * contents into the provided DataStorage.
   *
   * \param ds The DataStorage to which the data is added.
   * \return The set of added DataNodes to \c ds.
   *
   * This method may be overridden by implementations to create or
   * reconstructed a hierarchy of mitk::DataNode instances in the
   * provided mitk::DataStorage.
   *
   * \throws mitk::Exception
   */
  virtual DataStorage::SetOfObjects::Pointer Read(mitk::DataStorage& ds, volatile bool* interrupt = nullptr) = 0;

};

} // namespace mitk

MITK_DECLARE_SERVICE_INTERFACE(mitk::IFileReader, "org.mitk.IFileReader")

#endif /* IFileReader_H_HEADER_INCLUDED_C1E7E521 */
