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
