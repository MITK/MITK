/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __QMITK_IMAGE_AND_ROI_DATA_GENERATOR_BASE_H
#define __QMITK_IMAGE_AND_ROI_DATA_GENERATOR_BASE_H

#include "QmitkDataGeneratorBase.h"
#include <MitkImageStatisticsUIExports.h>

/*!
\brief QmitkDataGenerator
Class that ensures/generates the statistic data for given image nodes and ROI nodes (e.g. Segmentations or planarfigures).
The class implements the checking if statistic data is still up-to-date and triggers the generation,
if needed.
\tparam TDataGenerationRule a specific DataGenerationRule class, where details about (job) requirements and dependencies
are defined (e.g. which jobs needs to compute the statistic data)
The given dataStorage is used to save all generated data.
The TDataGenerationRule is executed on all combinations of image and ROI nodes.
If new data is generated, the signal NewDataAvailable is emitted.
@sa QmitkDataGeneratorBase
*/
class MITKIMAGESTATISTICSUI_EXPORT QmitkImageAndRoiDataGeneratorBase : public QmitkDataGeneratorBase
{
public:
  using Superclass = QmitkDataGeneratorBase;

  using NodeVectorType = std::vector<mitk::DataNode::ConstPointer>;

  NodeVectorType GetImageNodes() const;
  NodeVectorType GetROINodes() const;

public slots:
    /*! @brief Setter for image nodes
    */
    void SetImageNodes(const NodeVectorType& imageNodes);
    /*! @brief Setter for roi nodes
    */
    void SetROINodes(const NodeVectorType& roiNodes);

protected:
  QmitkImageAndRoiDataGeneratorBase(mitk::DataStorage::Pointer storage, QObject* parent = nullptr) : QmitkDataGeneratorBase(storage, parent) {};
  QmitkImageAndRoiDataGeneratorBase(QObject* parent = nullptr) : QmitkDataGeneratorBase(parent) {};

  using InputPairVectorType = Superclass::InputPairVectorType;
  
  bool NodeChangeIsRelevant(const mitk::DataNode *changedNode) const override;
  InputPairVectorType GetAllImageROICombinations() const override;

  NodeVectorType m_ImageNodes;
  NodeVectorType m_ROINodes;

  QmitkImageAndRoiDataGeneratorBase(const QmitkImageAndRoiDataGeneratorBase&) = delete;
  QmitkImageAndRoiDataGeneratorBase& operator = (const QmitkImageAndRoiDataGeneratorBase&) = delete;
};

#endif
