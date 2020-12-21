/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITK_POINT_SET_READER_SERVICE__H_
#define _MITK_POINT_SET_READER_SERVICE__H_

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
   * @internal
   *
   * @brief reads xml representations of mitk::PointSets from a file
   *
   * Reader for xml files containing one or multiple xml represenations of
   * mitk::PointSet. If multiple mitk::PointSet objects are stored in one file,
   * these are assigned to multiple BaseData objects.
   *
   * The reader is able to read the old 3D Pointsets without the "specification" and "timeseries" tags and the new 4D
   * Pointsets.
   *
   * @ingroup IO
   */
  class PointSetReaderService : public AbstractFileReader
  {
  public:
    PointSetReaderService();
    ~PointSetReaderService() override;

    using AbstractFileReader::Read;

  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    PointSetReaderService(const PointSetReaderService &other);

    mitk::BaseGeometry::Pointer ReadGeometry(tinyxml2::XMLElement *parentElement);

    mitk::PointSet::Pointer ReadPoints(mitk::PointSet::Pointer newPointSet,
                                       tinyxml2::XMLElement *currentTimeSeries,
                                       unsigned int currentTimeStep);

    PointSetReaderService *Clone() const override;
  };
}

#endif
