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


#ifndef __QMITK_WORKFLOW_BASE_H
#define __QMITK_WORKFLOW_BASE_H

//MITK
#include <mitkDataStorage.h>
#include <mitkDataNode.h>

//MITK-RTTB
#include <QmitkRTJobBase.h>

#include <MitkRTToolboxUIExports.h>

/*!
\brief QmitkTaskGenerationRuleBase
Base class for a Task generation rule
\details With task generation rule, we denote the process of defining a sequence of jobs (that encapsulate computation classes) to produce a desired output.
We require the following information:
- GetNextMissingJob(): determine the next job that has to be computed to compute the final result
- GetResultTypePredicate() and GetResultTypeBaseData(): information about the to-be-produced output data
\example to compute a DVH, dose and voxelization are required. If the voxelization is not available, a job has to be triggered to compute it.
*/

class MITKRTTOOLBOXUI_EXPORT QmitkTaskGenerationRuleBase
{
public:
	/*! @brief Returns if the (final or interim) result node is already available in the storage and up-to-date
    */
	bool IsResultAvailable(mitk::DataStorage::Pointer storage, mitk::DataNode::ConstPointer doseNode, mitk::DataNode::ConstPointer structNode) const;
	
  virtual mitk::DataNode::Pointer GetLatestResult(mitk::DataStorage::Pointer storage, mitk::DataNode::ConstPointer doseNode, mitk::DataNode::ConstPointer structNode) const = 0;
	/*! @brief Returns the next job that needs to be done in order to complete the workflow
    */
	virtual QmitkRTJobBase* GetNextMissingJob(mitk::DataStorage::Pointer storage, mitk::DataNode::ConstPointer doseNode, mitk::DataNode::ConstPointer structNode) const =0;
protected:
  /*! @brief Creates a data node for interim results
  @details It's the jobs responsibility to write the final results to the data of the DataNode.
  */
  mitk::DataNode::Pointer CreateDataNodeInterimResults(mitk::BaseData::Pointer data) const;
	/*! @brief Returns the voxelization job
      @details is required by two workflows (QmitkDoseStatisticsWorkflow and QmitkDVHWorkflow) and therefore provided in the base class
    */
	QmitkRTJobBase* GetVoxelizationJob(mitk::DataStorage::Pointer storage, mitk::DataNode::ConstPointer doseNode, mitk::DataNode::ConstPointer structNode) const;
  const mitk::DataNode* GetLatestVoxelization(mitk::DataStorage::Pointer storage, mitk::DataNode::ConstPointer doseNode, mitk::DataNode::ConstPointer structNode) const;
  mitk::BaseData::Pointer GetVoxelizedResultTypeBaseData() const;
};

#endif
