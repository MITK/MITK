/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTubeGraphDefinitions.h"

//============== XML const chars ===================

//==== Header information ====
const std::string mitk::TubeGraphDefinitions::XML_TUBEGRAPH_FILE = "tube_graph_file";
const std::string mitk::TubeGraphDefinitions::XML_FILE_VERSION = "file_version";
const std::string mitk::TubeGraphDefinitions::VERSION_STRING = "0.1";
const std::string mitk::TubeGraphDefinitions::ASCII_FILE = "ascii_file";
const std::string mitk::TubeGraphDefinitions::FILE_NAME = "file_name";

//==== Geometry details ====
const std::string mitk::TubeGraphDefinitions::XML_GEOMETRY = "geometry";

const std::string mitk::TubeGraphDefinitions::XML_MATRIX_XX = "xx";
const std::string mitk::TubeGraphDefinitions::XML_MATRIX_XY = "xy";
const std::string mitk::TubeGraphDefinitions::XML_MATRIX_XZ = "xz";
const std::string mitk::TubeGraphDefinitions::XML_MATRIX_YX = "yx";
const std::string mitk::TubeGraphDefinitions::XML_MATRIX_YY = "yy";
const std::string mitk::TubeGraphDefinitions::XML_MATRIX_YZ = "yz";
const std::string mitk::TubeGraphDefinitions::XML_MATRIX_ZX = "zx";
const std::string mitk::TubeGraphDefinitions::XML_MATRIX_ZY = "zy";
const std::string mitk::TubeGraphDefinitions::XML_MATRIX_ZZ = "zz";

const std::string mitk::TubeGraphDefinitions::XML_ORIGIN_X = "origin_x";
const std::string mitk::TubeGraphDefinitions::XML_ORIGIN_Y = "origin_y";
const std::string mitk::TubeGraphDefinitions::XML_ORIGIN_Z = "origin_z";

const std::string mitk::TubeGraphDefinitions::XML_SPACING_X = "spacing_x";
const std::string mitk::TubeGraphDefinitions::XML_SPACING_Y = "spacing_y";
const std::string mitk::TubeGraphDefinitions::XML_SPACING_Z = "spacing_z";

//==== TubeGraph structures ====
const std::string mitk::TubeGraphDefinitions::XML_VERTICES = "vertices";
const std::string mitk::TubeGraphDefinitions::XML_EDGES = "edges";
const std::string mitk::TubeGraphDefinitions::XML_LABELGROUPS = "labelgroups";
const std::string mitk::TubeGraphDefinitions::XML_ATTRIBUTIONS = "attributions";
const std::string mitk::TubeGraphDefinitions::XML_ANNOTATIONS = "annotations";
const std::string mitk::TubeGraphDefinitions::XML_VERTEX = "vertex";
const std::string mitk::TubeGraphDefinitions::XML_EDGE = "edge";
const std::string mitk::TubeGraphDefinitions::XML_ELEMENT = "element";
const std::string mitk::TubeGraphDefinitions::XML_LABELGROUP = "labelgroup";
const std::string mitk::TubeGraphDefinitions::XML_LABEL = "label";
const std::string mitk::TubeGraphDefinitions::XML_ATTRIBUTION = "attribut";
const std::string mitk::TubeGraphDefinitions::XML_ANNOTATION = "annotation";

//==== vertex properties ====
const std::string mitk::TubeGraphDefinitions::XML_VERTEX_ID = "vertex_id";

//==== edge properties ====
const std::string mitk::TubeGraphDefinitions::XML_EDGE_ID = "edge_id";
const std::string mitk::TubeGraphDefinitions::XML_EDGE_SOURCE_ID = "edge_source_id";
const std::string mitk::TubeGraphDefinitions::XML_EDGE_TARGET_ID = "edge_target_id";

//==== element properties ====
const std::string mitk::TubeGraphDefinitions::XML_ELEMENT_X = "element_x";
const std::string mitk::TubeGraphDefinitions::XML_ELEMENT_Y = "element_y";
const std::string mitk::TubeGraphDefinitions::XML_ELEMENT_Z = "element_z";
const std::string mitk::TubeGraphDefinitions::XML_ELEMENT_DIAMETER = "element_diameter";

//==== label group properties ====
const std::string mitk::TubeGraphDefinitions::XML_LABELGROUP_NAME = "labelgroup_name";

//==== label properties ====
const std::string mitk::TubeGraphDefinitions::XML_LABEL_NAME = "label_name";
const std::string mitk::TubeGraphDefinitions::XML_LABEL_VISIBILITY = "label_visible";
const std::string mitk::TubeGraphDefinitions::XML_LABEL_COLOR_R = "label_color_r";
const std::string mitk::TubeGraphDefinitions::XML_LABEL_COLOR_G = "label_color_g";
const std::string mitk::TubeGraphDefinitions::XML_LABEL_COLOR_B = "label_color_b";

//==== tube properties ====
const std::string mitk::TubeGraphDefinitions::XML_TUBE_ID_1 = "tube_id_1";
const std::string mitk::TubeGraphDefinitions::XML_TUBE_ID_2 = "tube_id_2";

//==== annotation properties ====
const std::string mitk::TubeGraphDefinitions::XML_ANNOTATION_NAME = "annotation_name";
const std::string mitk::TubeGraphDefinitions::XML_ANNOTATION_DESCRIPTION = "annotation_description";
