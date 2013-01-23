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

#ifndef __mitkConnectomicsNetworkDefinitions_h
#define __mitkConnectomicsNetworkDefinitions_h

namespace mitk
{

  /** \brief The XML consts for reading and writing
  */

  class ConnectomicsNetworkDefinitions
  {
  public:

    //============== XML const chars ===================

    //==== Header information ====
    static const char* XML_CONNECTOMICS_FILE;
    static const char* XML_FILE_VERSION;
    static const char* VERSION_STRING;
    static const char* ASCII_FILE;
    static const char* FILE_NAME;

    //==== Geometry details ====
    static const char* XML_GEOMETRY;

    static const char* XML_MATRIX_XX;
    static const char* XML_MATRIX_XY;
    static const char* XML_MATRIX_XZ;
    static const char* XML_MATRIX_YX;
    static const char* XML_MATRIX_YY;
    static const char* XML_MATRIX_YZ;
    static const char* XML_MATRIX_ZX;
    static const char* XML_MATRIX_ZY;
    static const char* XML_MATRIX_ZZ;

    static const char* XML_ORIGIN_X;
    static const char* XML_ORIGIN_Y;
    static const char* XML_ORIGIN_Z;

    static const char* XML_SPACING_X;
    static const char* XML_SPACING_Y;
    static const char* XML_SPACING_Z;

    //==== Connectomics structures ====
    static const char* XML_VERTICES;
    static const char* XML_EDGES;
    static const char* XML_VERTEX;
    static const char* XML_EDGE;

    //==== Connectomics properties ====
    static const char* XML_VERTEX_ID;
    static const char* XML_VERTEX_LABEL;
    static const char* XML_VERTEX_X;
    static const char* XML_VERTEX_Y;
    static const char* XML_VERTEX_Z;
    static const char* XML_EDGE_ID;
    static const char* XML_EDGE_SOURCE_ID;
    static const char* XML_EDGE_TARGET_ID;
    static const char* XML_EDGE_WEIGHT_ID;

  private:
    ConnectomicsNetworkDefinitions();
    ~ConnectomicsNetworkDefinitions();

  };

} //namespace MITK

#endif // __mitkConnectomicsNetworkDefinitions_h
