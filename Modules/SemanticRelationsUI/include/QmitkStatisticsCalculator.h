/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKSTATISTICSCALCULATOR_H
#define QMITKSTATISTICSCALCULATOR_H

// mitk semantic relations UI
#include "MitkSemanticRelationsUIExports.h"

// mitk semantic relations module
#include <mitkLesionData.h>
#include <mitkSemanticTypes.h>

// mitk core
#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>

// mitk image statistics ui module
#include <QmitkImageStatisticsCalculationJob.h>

/*
* @brief This class provides functions to compute the lesion volume of a given lesion.
*        A lesion can be defined by a specific segmentation at each control-point - information type pair.
*        This segmentation and its parent image will be used inside the private 'GetSegmentationMaskVolume' function.
*        This function in turn uses the image statistics module (the 'ImageStatisticsContainerManager') to retrieve
*        the specific statistics values from a statistics node. However, if the statistics are not found,
*        a new 'QmitkImageStatisticsCalculationJob' is started. If the job is finished and the statistics are calculated,
*        a new statistics node is added to the data storage (or an existing statistics data node is updated).
*/
class MITKSEMANTICRELATIONSUI_EXPORT QmitkStatisticsCalculator : public QObject
{
  Q_OBJECT

public:

  QmitkStatisticsCalculator();
  ~QmitkStatisticsCalculator() override;

  void SetDataStorage(mitk::DataStorage* dataStorage) { m_DataStorage = dataStorage; }
  /**
  * @brief Compute and store lesion volume for all available control points and information types.
  *
  * @param  lesionData          The lesion data that holds the lesion and will hold the additional lesion data.
  * @param  caseID              The current case ID.
  */
  void ComputeLesionVolume(mitk::LesionData& lesionData, const mitk::SemanticTypes::CaseID& caseID);

private:

  /**
  * @brief
  *
  *
  */
  double GetSegmentationMaskVolume(mitk::DataNode::Pointer imageNode, mitk::DataNode::Pointer segmentationNode);

  void OnStatisticsCalculationEnds();

  QmitkImageStatisticsCalculationJob* m_CalculationJob;
  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  mitk::DataNode::Pointer m_ImageNode;
  mitk::DataNode::Pointer m_SegmentationNode;
  double m_MaskVolume;

};

#endif // QMITKSTATISTICSCALCULATOR_H
