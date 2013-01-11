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
//#ifndef _MITK_ConnectomicsConstantsManager_CPP
//#define _MITK_ConnectomicsConstantsManager_CPP

#include "mitkConnectomicsConstantsManager.h"

//============== String and other constants ===================

//==== Error messages ====
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_ERROR_TRIED_TO_ACCESS_INVALID_HISTOGRAM = "Tried to access invalid histogram.";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_ERROR_PASSED_NEGATIVE_INDEX_TO_HISTOGRAM = "Passed negative index to histogram.";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_ERROR_OUTSIDE_INTEGER_RANGE = "Tried to access histogram vector outside integer range.";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_ERROR_BEYOND_SCOPE = "Tried to access histogram vector for value beyond scope: ";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_ERROR_INVALID_MAPPING = "Invalid mapping strategy selected.";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_ERROR_INVALID_DIMENSION_NEED_3 = "Invalid dimension, need dimension 3.";

//==== Warnings ====
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_ZERO_DISTANCE_NODES = "There are nodes which are not distance 0 to themselves.";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_UNIMPLEMENTED_FEATURE = "You are trying to use an as of yet unimplemented feature.";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_MORE_POINTS_THAN_PRESENT = "Trying to estimate using more points than are present.";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_ESTIMATING_LESS_THAN_2 = "Trying to estimate using less than two points.";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_ESTIMATING_BEYOND_END = "Trying to estimate using points beyond track end.";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_ESTIMATING_BEYOND_START = "Trying to estimate using points beyond track start.";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_DID_NOT_FIND_WHITE = "Did not find a non white matter label.";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_NOT_EXTEND_TO_WHITE = "Could not extend to non white matter.";

//==== Information ====
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_INFO_NETWORK_CREATED = "Network has been created.";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_NETWORK_NOT_VALID = "Network not valid.";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_NETWORK_DISCONNECTED = "Network is disconnected.";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_CAN_NOT_COMPUTE_EFFICIENCY = "Can not compute efficiency. ";

//==== GUI ====
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH = "-";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_CONNECTOMICS = "Connectomics";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_CONNECTOMICS_CREATION = "Connectomics Creation";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_SELECTION_WARNING = "Please load and select exactly one parcellation image and one fiber image before starting network creation.";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_ONLY_PARCELLATION_SELECTION_WARNING = "Please load and select one parcellation image.";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_PERFORMING_IMAGE_PROCESSING_FOR_IMAGE = "Performing image processing for image ";

//==== Properties ====
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_PROPERTY_DEFAULT_RGBA_NAME = "rgbaImage";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_PROPERTY_NAME = "name";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_PROPERTY_VOLUMERENDERING = "volumerendering";
const char* mitk::ConnectomicsConstantsManager::CONNECTOMICS_PROPERTY_DEFAULT_CNF_NAME = "Connectomics network";

//#endif /* _MITK_ConnectomicsConstantsManager_CPP */
