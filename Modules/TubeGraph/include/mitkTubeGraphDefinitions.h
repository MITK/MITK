/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __mitkTubeGraphDefinitions_h
#define __mitkTubeGraphDefinitions_h

#include <string>

namespace mitk
{
  /** \brief The XML consts for reading and writing
  */

  class TubeGraphDefinitions
  {
  public:
    //============== XML const chars ===================

    //==== Header information ====
    static const std::string XML_TUBEGRAPH_FILE;
    static const std::string XML_FILE_VERSION;
    static const std::string VERSION_STRING;
    static const std::string ASCII_FILE;
    static const std::string FILE_NAME;

    //==== Geometry details ====
    static const std::string XML_GEOMETRY;

    static const std::string XML_MATRIX_XX;
    static const std::string XML_MATRIX_XY;
    static const std::string XML_MATRIX_XZ;
    static const std::string XML_MATRIX_YX;
    static const std::string XML_MATRIX_YY;
    static const std::string XML_MATRIX_YZ;
    static const std::string XML_MATRIX_ZX;
    static const std::string XML_MATRIX_ZY;
    static const std::string XML_MATRIX_ZZ;

    static const std::string XML_ORIGIN_X;
    static const std::string XML_ORIGIN_Y;
    static const std::string XML_ORIGIN_Z;

    static const std::string XML_SPACING_X;
    static const std::string XML_SPACING_Y;
    static const std::string XML_SPACING_Z;

    //==== tube graph structures ====
    static const std::string XML_VERTICES;
    static const std::string XML_EDGES;
    static const std::string XML_LABELGROUPS;
    static const std::string XML_ATTRIBUTIONS;
    static const std::string XML_ANNOTATIONS;
    static const std::string XML_VERTEX;
    static const std::string XML_EDGE;
    static const std::string XML_ELEMENT;
    static const std::string XML_LABELGROUP;
    static const std::string XML_LABEL;
    static const std::string XML_ATTRIBUTION;
    static const std::string XML_ANNOTATION;

    //==== vertex properties ====
    static const std::string XML_VERTEX_ID;

    //==== edge properties ====
    static const std::string XML_EDGE_ID;
    static const std::string XML_EDGE_SOURCE_ID;
    static const std::string XML_EDGE_TARGET_ID;

    //==== element properties ====

    static const std::string XML_ELEMENT_X;
    static const std::string XML_ELEMENT_Y;
    static const std::string XML_ELEMENT_Z;
    static const std::string XML_ELEMENT_DIAMETER;

    //==== label group properties ====
    static const std::string XML_LABELGROUP_NAME;

    //==== label properties ====
    static const std::string XML_LABEL_NAME;
    static const std::string XML_LABEL_VISIBILITY;
    static const std::string XML_LABEL_COLOR_R;
    static const std::string XML_LABEL_COLOR_G;
    static const std::string XML_LABEL_COLOR_B;

    //==== tube properties ====
    static const std::string XML_TUBE_ID_1;
    static const std::string XML_TUBE_ID_2;

    //==== annotation properties ====
    static const std::string XML_ANNOTATION_NAME;
    static const std::string XML_ANNOTATION_DESCRIPTION;

  private:
    TubeGraphDefinitions();
    ~TubeGraphDefinitions();
  };

} // namespace MITK

#endif // __mitkTubeGraphDefinitions_h
