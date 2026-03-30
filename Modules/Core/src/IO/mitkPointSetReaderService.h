/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPointSetReaderService_h
#define mitkPointSetReaderService_h

// MITK
#include <mitkAbstractFileReader.h>
#include <mitkPointSet.h>

namespace tinyxml2
{
  class XMLElement;
}

namespace mitk
{
  /**
   * \internal
   *
   * \brief Reads XML representations of mitk::PointSets from a file.
   *
   * Reader for XML files containing one or multiple serialized mitk::PointSet
   * objects. If multiple PointSets are stored in one file, each is returned
   * as a separate BaseData object.
   *
   * The reader supports both the old 3D format (without "specification" and
   * "timeseries" tags) and the new 4D format with time series support.
   *
   * \ingroup IO
   * \sa PointSetWriterService
   * \sa AbstractFileReader
   */
  class PointSetReaderService : public AbstractFileReader
  {
  public:
    PointSetReaderService();
    ~PointSetReaderService() override;

    using AbstractFileReader::Read;

  protected:
    /**
     * \brief Read one or more PointSets from the XML input file.
     * \return A vector of BaseData objects, one per PointSet found in the file.
     */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    PointSetReaderService(const PointSetReaderService &other);

    /**
     * \brief Read a Geometry3D from an XML element.
     * \param[in] parentElement The XML element containing geometry data.
     * \return The deserialized BaseGeometry.
     */
    mitk::BaseGeometry::Pointer ReadGeometry(tinyxml2::XMLElement *parentElement);

    /**
     * \brief Read point data from an XML time series element.
     * \param[in] newPointSet The PointSet to populate.
     * \param[in] currentTimeSeries The XML element for the current time series.
     * \param[in] currentTimeStep The time step index.
     * \return The populated PointSet.
     */
    mitk::PointSet::Pointer ReadPoints(mitk::PointSet::Pointer newPointSet,
                                       tinyxml2::XMLElement *currentTimeSeries,
                                       unsigned int currentTimeStep);

    PointSetReaderService *Clone() const override;
  };
}

#endif
