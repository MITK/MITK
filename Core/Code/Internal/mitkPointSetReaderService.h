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

#ifndef _MITK_POINT_SET_READER__H_
#define _MITK_POINT_SET_READER__H_

// ITK
#include <itkLightObject.h>
#include <itkObject.h>

// MITK
#include <mitkAbstractFileReader.h>

// Old, delete if necessary
#include <mitkPointSetSource.h>
#include <mitkFileReader.h>
#include <string>
#include <stack>
#include <vtkXMLParser.h>
#include <tinyxml.h>

namespace mitk
{
  /**
  * @brief reads xml representations of mitk::PointSets from a file
  *
  * Reader for xml files containing one or multiple xml represenations of
  * mitk::PointSets. If multiple mitk::PointSets are stored in one file,
  * these are assigned to multiple outputs of the filter. The number of point
  * sets which have be read can be retrieven by a call to GetNumberOfOutputs()
  * after the pipeline update().
  * The reader is able to read the old 3D Pointsets without the "specification" and "timeseries" tags and the new 4D Pointsets.
  * @note loading point sets from multiple files according to a given file pattern
  * is not yet supported!
  * @ingroup PSIO
  * @ingroup IO
  */
  class PointSetReaderService: public AbstractFileReader
  {
  public:

    /**
    * Constructor
    */
    PointSetReaderService();

    /**
    * Virtual destructor
    */
    virtual ~PointSetReaderService();

    using AbstractFileReader::Read;

    virtual std::vector< itk::SmartPointer<BaseData> > Read(std::istream& stream);

  protected:

    PointSetReaderService(const PointSetReaderService& other);

    virtual mitk::PointSet::Pointer ReadPoint(mitk::PointSet::Pointer newPointSet,
      TiXmlElement* currentTimeSeries, unsigned int currentTimeStep);

    virtual mitk::IFileReader* Clone() const;
  };
}

#endif
