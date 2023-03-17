/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkResultNodeGenerationHelper_h
#define mitkResultNodeGenerationHelper_h

#include "mitkDataNode.h"
#include "mitkMAPRegistrationWrapper.h"

#include "MitkMatchPointRegistrationExports.h"

namespace mitk
{
    /**Method generates a proper result node for the given registration wrapper.
    @param nodeName Name of the result node
    @param resultReg Pointer to the registration wrapper that should be data of the node.
    @param algorithmUID UID string of the algorithm used to generate the result.
    @param movingDataUID UID string of the data used as moving input for the registration algorithm.
    @param targetDataUID UID string of the data used as moving input for the registration algorithm.
    @pre registration must point to a valid instance
    @result Pointer to a data node with all properties properly set.*/
    MITKMATCHPOINTREGISTRATION_EXPORT mitk::DataNode::Pointer generateRegistrationResultNode(const std::string& nodeName, mitk::MAPRegistrationWrapper::Pointer resultReg, const std::string& algorithmUID, const std::string& movingDataUID, const std::string& targetDataUID);

    /**Method generates a proper result node for the given registration wrapper.
    @param nodeName Name of the result node
    @param mappedData Pointer to the mapped data that should be data of the node.
    @param regUID UID string of the registration used to map the data.
    @param inputDataUID UID string of the data used as input for the mapping.
    @param refinedGeometry Indicates if the data was really mapped or the geometry was refined.
    @param interpolator Name of the used interpolation strategy.
    @pre mappedData must point to a valid instance
    @result Pointer to a data node with all properties properly set.*/
    MITKMATCHPOINTREGISTRATION_EXPORT mitk::DataNode::Pointer generateMappedResultNode(const std::string& nodeName, mitk::BaseData::Pointer mappedData, const std::string& regUID, const std::string& inputDataUID, const bool refinedGeometry, const std::string& interpolator = "Unkown");

}

#endif
