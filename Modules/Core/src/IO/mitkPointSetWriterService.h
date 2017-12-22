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

#ifndef _MITK_POINT_SET_WRITER_SERVICE__H_
#define _MITK_POINT_SET_WRITER_SERVICE__H_

#include <mitkAbstractFileWriter.h>
#include <mitkPointSet.h>

class TiXmlElement;

namespace mitk
{
  class Geometry3D;

  /**
   * @internal
   *
   * @brief XML-based writer for mitk::PointSets
   *
   * XML-based writer for mitk::PointSet. Multiple PointSets can be written in
   * a single XML file by simply setting multiple inputs to the filter.
   *
   * @todo This class would merit a XML library for maintainability or a denser format for performance.
   *
   * @ingroup IO
   */
  class PointSetWriterService : public AbstractFileWriter
  {
  public:
    PointSetWriterService();
    ~PointSetWriterService() override;

    using AbstractFileWriter::Write;
    void Write() override;

  private:
    PointSetWriterService(const PointSetWriterService &other);

    mitk::PointSetWriterService *Clone() const override;

    template <typename T>
    std::string ConvertToString(T value);

    TiXmlElement *ToXML(const mitk::PointSet *pointSet);

    static const std::string XML_POINT_SET;
    static const std::string XML_TIME_SERIES;
    static const std::string XML_TIME_SERIES_ID;
    static const std::string XML_POINT_SET_FILE;
    static const std::string XML_FILE_VERSION;
    static const std::string XML_POINT;
    static const std::string XML_SPEC;
    static const std::string XML_ID;
    static const std::string XML_X;
    static const std::string XML_Y;
    static const std::string XML_Z;
    static const std::string VERSION_STRING;
  };
}

#endif
