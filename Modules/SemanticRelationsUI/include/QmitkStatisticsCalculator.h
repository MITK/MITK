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

#ifndef QMITKSTATISTICSCALCULATOR_H
#define QMITKSTATISTICSCALCULATOR_H

// mitk semantic relations UI
#include "MitkSemanticRelationsUIExports.h"

// mitk semantic relations module
#include <mitkLesionData.h>
#include <mitkSemanticTypes.h>

// mitk core
#include <mitkDataStorage.h>
#include <mitkDataNode.h>
#include <mitkWeakPointer.h>

// mitk image statistics ui module
#include <QmitkImageStatisticsCalculationJob.h>

// itk
#include <itkImage.h>

/*
* @brief 
*/
class MITKSEMANTICRELATIONSUI_EXPORT QmitkStatisticsCalculator : public QObject
{
  Q_OBJECT

public:

  QmitkStatisticsCalculator();
  ~QmitkStatisticsCalculator();

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

  QmitkImageStatisticsCalculationJob * m_CalculationJob;
  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  mitk::DataNode::Pointer m_ImageNode;
  mitk::DataNode::Pointer m_SegmentationNode;
  double m_MaskVolume;

};

#endif // QMITKSTATISTICSCALCULATOR_H
