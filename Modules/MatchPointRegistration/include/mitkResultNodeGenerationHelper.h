/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkResultNodeGenerationHelper_h
#define mitkResultNodeGenerationHelper_h

#include <mitkDataNode.h>
#include <mitkMAPRegistrationWrapper.h>

#include <MitkMatchPointRegistrationExports.h>

namespace mitk
{
    /**
     * \brief Generate a result data node for a registration wrapper.
     *
     * \param[in] nodeName Name of the result node.
     * \param[in] resultReg Pointer to the registration wrapper (must not be nullptr).
     * \param[in] algorithmUID UID string of the algorithm used to generate the result.
     * \param[in] movingDataUID UID string of the data used as moving input.
     * \param[in] targetDataUID UID string of the data used as target input.
     * \return Pointer to a data node with all properties properly set.
     */
    MITKMATCHPOINTREGISTRATION_EXPORT mitk::DataNode::Pointer generateRegistrationResultNode(const std::string& nodeName, mitk::MAPRegistrationWrapper::Pointer resultReg, const std::string& algorithmUID, const std::string& movingDataUID, const std::string& targetDataUID);

    /**
     * \brief Generate a result data node for mapped data.
     *
     * \param[in] nodeName Name of the result node.
     * \param[in] mappedData Pointer to the mapped data (must not be nullptr).
     * \param[in] regUID UID string of the registration used to map the data.
     * \param[in] inputDataUID UID string of the data used as input for the mapping.
     * \param[in] refinedGeometry True if the geometry was refined rather than fully mapped.
     * \param[in] interpolator Name of the interpolation strategy used.
     * \return Pointer to a data node with all properties properly set.
     */
    MITKMATCHPOINTREGISTRATION_EXPORT mitk::DataNode::Pointer generateMappedResultNode(const std::string& nodeName, mitk::BaseData::Pointer mappedData, const std::string& regUID, const std::string& inputDataUID, const bool refinedGeometry, const std::string& interpolator = "Unkown");

}

#endif
