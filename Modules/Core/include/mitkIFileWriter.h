/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIFileWriter_h
#define mitkIFileWriter_h

#include <mitkIFileIO.h>

#include <mitkServiceInterface.h>

namespace mitk
{
  class BaseData;
}

namespace mitk
{
  /**
   * \ingroup IO
   * \ingroup MicroServices_Interfaces
   *
   * \brief The common interface of all MITK file writers.
   *
   * Implementations of this interface must be registered as a service
   * to make themselves available via the service registry. If the
   * implementation is stateful, the service should be registered using
   * a PrototypeServiceFactory.
   *
   * The file writer implementation is associated with a mime-type, specified
   * in the service property PROP_MIMETYPE(), and a mitk::BaseData sub-class
   * as specified in the PROP_BASEDATA_TYPE() service property.
   * The specified mime-type should have a corresponding CustomMimeType service
   * object, registered by the writer or some other party.
   *
   * It is recommended to derive new implementations from AbstractFileWriter or
   * AbstractFileIO (if both reader and writer are implemented),
   * which provide correct service registration semantics.
   *
   * A more in-depth explanation is available on the concepts pages in the developer manual.
   *
   * \sa AbstractFileWriter
   * \sa AbstractFileIO
   * \sa CustomMimeType
   * \sa FileWriterSelector
   * \sa IFileReader
   */
  struct MITKCORE_EXPORT IFileWriter : public IFileIO
  {
    ~IFileWriter() override;

    /**
     * \brief Set the input data for writing.
     * \param[in] data Pointer to the BaseData object to be written.
     */
    virtual void SetInput(const BaseData *data) = 0;

    /**
     * \brief Get the input data set via SetInput().
     * \return Pointer to the BaseData object, or \c nullptr if none was set.
     */
    virtual const BaseData *GetInput() const = 0;

    /**
     * \brief Set the output location.
     * \param[in] location A file-system path where data shall be written.
     *
     * The location argument specifies a file-system path where the input
     * data must be written. This method must remove any previously set
     * output stream.
     */
    virtual void SetOutputLocation(const std::string &location) = 0;

    /**
     * \brief Get the current output location.
     * \return The output location string.
     *
     * If no stream is set (i.e. GetOutputStream() returns \c nullptr), the
     * returned location string is required to represent a file-system path.
     */
    virtual std::string GetOutputLocation() const = 0;

    /**
     * \brief Set an output stream for writing.
     * \param[in] location A custom label for the output stream, or a file-system path.
     * \param[in] os The output stream, or \c nullptr to clear the stream.
     *
     * If \c os is \c nullptr, this clears the current output stream and \c location
     * is interpreted as a file-system path. Otherwise, \c location is a custom
     * label describing the output stream \c os.
     */
    virtual void SetOutputStream(const std::string &location, std::ostream *os) = 0;

    /**
     * \brief Get the output stream.
     * \return The currently set output stream, or \c nullptr if no stream was set.
     */
    virtual std::ostream *GetOutputStream() const = 0;

    /**
     * \brief Write the input data to the output location or stream.
     *
     * If GetOutputStream() returns a non-null value, this method must use
     * the returned stream object to write the data set via SetInput(). If no
     * output stream was set, the data must be written to the path returned
     * by GetOutputLocation().
     *
     * \throws mitk::Exception if the data could not be written.
     */
    virtual void Write() = 0;

    /**
     * \brief Service property name for the supported mitk::BaseData sub-class.
     *
     * The property value must be of type \c std::string and should correspond
     * to the class name returned by BaseData::GetNameOfClass().
     *
     * \return The property name string.
     */
    static std::string PROP_BASEDATA_TYPE();
  };

} // namespace mitk

MITK_DECLARE_SERVICE_INTERFACE(mitk::IFileWriter, "org.mitk.IFileWriter")

#endif
