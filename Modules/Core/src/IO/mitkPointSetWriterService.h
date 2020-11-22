/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITK_POINT_SET_WRITER_SERVICE__H_
#define _MITK_POINT_SET_WRITER_SERVICE__H_

#include <mitkAbstractFileWriter.h>
#include <mitkPointSet.h>

namespace tinyxml2
{
  class XMLDocument;
  class XMLElement;
}

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

    tinyxml2::XMLElement *ToXML(tinyxml2::XMLDocument &doc, const mitk::PointSet *pointSet);

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
