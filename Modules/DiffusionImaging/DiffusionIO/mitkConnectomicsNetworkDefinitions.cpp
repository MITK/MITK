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

#include "mitkConnectomicsNetworkDefinitions.h"

//============== XML const chars ===================

//==== Header information ====
const char* mitk::ConnectomicsNetworkDefinitions::XML_CONNECTOMICS_FILE = "connectomics_network_file" ;
const char* mitk::ConnectomicsNetworkDefinitions::XML_FILE_VERSION = "file_version" ;
const char* mitk::ConnectomicsNetworkDefinitions::VERSION_STRING = "0.1" ;
const char* mitk::ConnectomicsNetworkDefinitions::ASCII_FILE = "ascii_file" ;
const char* mitk::ConnectomicsNetworkDefinitions::FILE_NAME = "file_name" ;

//==== Geometry details ====
const char* mitk::ConnectomicsNetworkDefinitions::XML_GEOMETRY = "geometry";

const char* mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_XX = "xx";
const char* mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_XY = "xy";
const char* mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_XZ = "xz";
const char* mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_YX = "yx";
const char* mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_YY = "yy";
const char* mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_YZ = "yz";
const char* mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_ZX = "zx";
const char* mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_ZY = "zy";
const char* mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_ZZ = "zz";

const char* mitk::ConnectomicsNetworkDefinitions::XML_ORIGIN_X = "origin_x";
const char* mitk::ConnectomicsNetworkDefinitions::XML_ORIGIN_Y = "origin_y";
const char* mitk::ConnectomicsNetworkDefinitions::XML_ORIGIN_Z = "origin_z";

const char* mitk::ConnectomicsNetworkDefinitions::XML_SPACING_X = "spacing_x";
const char* mitk::ConnectomicsNetworkDefinitions::XML_SPACING_Y = "spacing_y";
const char* mitk::ConnectomicsNetworkDefinitions::XML_SPACING_Z = "spacing_z";

//==== Connectomics structures ====
const char* mitk::ConnectomicsNetworkDefinitions::XML_VERTICES = "vertices";
const char* mitk::ConnectomicsNetworkDefinitions::XML_EDGES = "edges";
const char* mitk::ConnectomicsNetworkDefinitions::XML_VERTEX = "vertex";
const char* mitk::ConnectomicsNetworkDefinitions::XML_EDGE = "edge";

//==== Connectomics properties ====
const char* mitk::ConnectomicsNetworkDefinitions::XML_VERTEX_ID = "v_id";
const char* mitk::ConnectomicsNetworkDefinitions::XML_VERTEX_LABEL = "v_label";
const char* mitk::ConnectomicsNetworkDefinitions::XML_VERTEX_X = "v_x";
const char* mitk::ConnectomicsNetworkDefinitions::XML_VERTEX_Y = "v_y";
const char* mitk::ConnectomicsNetworkDefinitions::XML_VERTEX_Z = "v_z";
const char* mitk::ConnectomicsNetworkDefinitions::XML_EDGE_ID = "e_id";
const char* mitk::ConnectomicsNetworkDefinitions::XML_EDGE_SOURCE_ID = "e_s_id";
const char* mitk::ConnectomicsNetworkDefinitions::XML_EDGE_TARGET_ID = "e_t_id";
const char* mitk::ConnectomicsNetworkDefinitions::XML_EDGE_WEIGHT_ID = "e_weight";
