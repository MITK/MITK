/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGeometryDataReaderService_h
#define mitkGeometryDataReaderService_h

// MITK
#include <mitkAbstractFileReader.h>
#include <mitkGeometryData.h>

namespace mitk
{
  /**
   * \internal
   *
   * \brief Reads XML representations of mitk::GeometryData from a file/stream.
   *
   * To be used via IOUtil.
   *
   * Reader for XML files containing one or multiple XML representations of
   * mitk::GeometryData. If multiple mitk::GeometryData objects are stored in one file,
   * these are assigned to multiple BaseData objects.
   *
   * \sa Geometry3DToXML
   *
   * \ingroup IO
   */
  class GeometryDataReaderService : public AbstractFileReader
  {
  public:
    /** \brief Default constructor. Registers the reader service for geometry data MIME type. */
    GeometryDataReaderService();

    /** \brief Destructor. */
    ~GeometryDataReaderService() override;

    using AbstractFileReader::Read;

    /**
     * \brief Provide the MIME type for geometry data reader and writer.
     * \return The custom MIME type for geometry data files.
     */
    static CustomMimeType GEOMETRY_DATA_MIMETYPE();

  protected:
    /** \brief Read geometry data from the configured input.
     * \return A vector of BaseData objects containing the deserialized GeometryData.
     * \throws mitk::Exception if the input cannot be parsed or is empty.
     */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    /** \brief Copy constructor. */
    GeometryDataReaderService(const GeometryDataReaderService &other);

    /** \brief Clone this reader instance. */
    GeometryDataReaderService *Clone() const override;
  };
}

#endif
