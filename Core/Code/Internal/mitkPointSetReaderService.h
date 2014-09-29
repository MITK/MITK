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

#ifndef _MITK_POINT_SET_READER_SERVICE__H_
#define _MITK_POINT_SET_READER_SERVICE__H_

// MITK
#include <mitkAbstractFileReader.h>
#include <mitkPointSet.h>

class TiXmlElement;

namespace mitk
{

/**
 * @brief reads xml representations of mitk::PointSets from a file
 *
 * Reader for xml files containing one or multiple xml represenations of
 * mitk::PointSet. If multiple mitk::PointSet objects are stored in one file,
 * these are assigned to multiple BaseData objects.
 *
 * The reader is able to read the old 3D Pointsets without the "specification" and "timeseries" tags and the new 4D Pointsets.
 *
 * @ingroup IO
 */
class PointSetReaderService: public AbstractFileReader
{
public:

  PointSetReaderService();
  virtual ~PointSetReaderService();

  using AbstractFileReader::Read;
  virtual std::vector< itk::SmartPointer<BaseData> > Read();

private:

  PointSetReaderService(const PointSetReaderService& other);

  mitk::PointSet::Pointer ReadPoint(mitk::PointSet::Pointer newPointSet,
                                    TiXmlElement* currentTimeSeries, unsigned int currentTimeStep);

  virtual PointSetReaderService* Clone() const;
};

}

#endif
