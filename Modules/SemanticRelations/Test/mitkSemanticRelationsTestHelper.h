/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// mitk core
#include <mitkDataNode.h>

namespace mitk
{
  namespace SemanticRelationsTestHelper
  {
    //////////////////////////////////////////////////////////////////////////
    // VALID DATA NODES
    //////////////////////////////////////////////////////////////////////////
    DataNode::Pointer GetPatientOneCTImage();

    DataNode::Pointer GetPatientOneMRImage();

    DataNode::Pointer GetPatientOneOtherCTImage();

    DataNode::Pointer GetPatientTwoPETImage();

    DataNode::Pointer GetPatientTwoSegmentation();

    DataNode::Pointer GetPatientThreeCTImage();

    DataNode::Pointer GetPatientThreeCTSegmentation();

    DataNode::Pointer GetPatientThreeMRImage();

    DataNode::Pointer GetPatientThreeMRSegmentation();

    //////////////////////////////////////////////////////////////////////////
    // INVALID DATA NODES
    //////////////////////////////////////////////////////////////////////////
    /**
    * @brief Date is 0x0008, 0x0022 (AcquisitionDate)
    */
    DataNode::Pointer GetInvalidDate();
    /**
    * @brief Modality is 0x0008, 0x0060(Modality)
    */
    DataNode::Pointer GetInvalidModality();
    /**
    * @brief ID is 0x0020, 0x000e (SeriesInstanceUID)
    */
    DataNode::Pointer GetInvalidID();
    /**
    * @brief CaseID is 0x0010, 0x0010 (PatientName)
    */
    DataNode::Pointer GetInvalidCaseID();

    //////////////////////////////////////////////////////////////////////////
    // AUXILIARY FUNCTIONS
    //////////////////////////////////////////////////////////////////////////
    void ClearRelationStorage();

  } // end SemanticRelationsTestHelper
} // end mitk
