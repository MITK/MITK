
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

#ifndef _MITK_ConnectomicsConstantsManager_H
#define _MITK_ConnectomicsConstantsManager_H

#include "MitkDiffusionImagingExports.h"

namespace mitk
{

  /** \brief The XML consts for reading and writing
  */

  class MitkDiffusionImaging_EXPORT ConnectomicsConstantsManager
  {
  public:

    //============== String and other constants ===================

    //==== Error messages ====
    static const char* CONNECTOMICS_ERROR_TRIED_TO_ACCESS_INVALID_HISTOGRAM;
    static const char* CONNECTOMICS_ERROR_PASSED_NEGATIVE_INDEX_TO_HISTOGRAM;
    static const char* CONNECTOMICS_ERROR_OUTSIDE_INTEGER_RANGE;
    static const char* CONNECTOMICS_ERROR_BEYOND_SCOPE;
    static const char* CONNECTOMICS_ERROR_INVALID_MAPPING;
    static const char* CONNECTOMICS_ERROR_INVALID_DIMENSION_NEED_3;

    //==== Warnings ====
    static const char* CONNECTOMICS_WARNING_ZERO_DISTANCE_NODES;
    static const char* CONNECTOMICS_WARNING_UNIMPLEMENTED_FEATURE;
    static const char* CONNECTOMICS_WARNING_MORE_POINTS_THAN_PRESENT;
    static const char* CONNECTOMICS_WARNING_ESTIMATING_LESS_THAN_2;
    static const char* CONNECTOMICS_WARNING_ESTIMATING_BEYOND_END;
    static const char* CONNECTOMICS_WARNING_ESTIMATING_BEYOND_START;
    static const char* CONNECTOMICS_WARNING_DID_NOT_FIND_WHITE;
    static const char* CONNECTOMICS_WARNING_NOT_EXTEND_TO_WHITE;

    //==== Information ====
    static const char* CONNECTOMICS_WARNING_INFO_NETWORK_CREATED;
    static const char* CONNECTOMICS_WARNING_NETWORK_NOT_VALID;
    static const char* CONNECTOMICS_WARNING_NETWORK_DISCONNECTED;
    static const char* CONNECTOMICS_WARNING_CAN_NOT_COMPUTE_EFFICIENCY;

    //==== GUI ====
    static const char* CONNECTOMICS_GUI_DASH;
    static const char* CONNECTOMICS_GUI_CONNECTOMICS;
    static const char* CONNECTOMICS_GUI_CONNECTOMICS_CREATION;
    static const char* CONNECTOMICS_GUI_SELECTION_WARNING;
    static const char* CONNECTOMICS_GUI_ONLY_PARCELLATION_SELECTION_WARNING;
    static const char* CONNECTOMICS_GUI_PERFORMING_IMAGE_PROCESSING_FOR_IMAGE;

    //==== Properties ====
    static const char* CONNECTOMICS_PROPERTY_DEFAULT_RGBA_NAME;
    static const char* CONNECTOMICS_PROPERTY_NAME;
    static const char* CONNECTOMICS_PROPERTY_VOLUMERENDERING;
    static const char* CONNECTOMICS_PROPERTY_DEFAULT_CNF_NAME;


  private:
    ConnectomicsConstantsManager();
    ~ConnectomicsConstantsManager();

  };

} //namespace MITK

// include cpp

//#include <mitkConnectomicsConstantsManager.cpp>

#endif /* _MITK_ConnectomicsConstantsManager_H */
