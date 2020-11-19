/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkImageStatisticsDataGenerator_h
#define QmitkImageStatisticsDataGenerator_h

#include "QmitkImageAndRoiDataGeneratorBase.h"

#include <MitkImageStatisticsUIExports.h>

/**
Generates ImageStatisticContainers by using QmitkImageStatisticsCalculationRunnables for each pair if image and ROIs and ensures their
validity.
It also encodes the HistogramNBins and IgnoreZeroValueVoxel as properties to the results as these settings are important criteria for
discreminating statistics results.
For more details of how the generation is done see QmitkDataGenerationBase.
*/
class MITKIMAGESTATISTICSUI_EXPORT QmitkImageStatisticsDataGenerator : public QmitkImageAndRoiDataGeneratorBase
{
public:
  QmitkImageStatisticsDataGenerator(mitk::DataStorage::Pointer storage, QObject* parent = nullptr) : QmitkImageAndRoiDataGeneratorBase(storage, parent) {};
  QmitkImageStatisticsDataGenerator(QObject* parent = nullptr) : QmitkImageAndRoiDataGeneratorBase(parent) {};

  bool IsValidResultAvailable(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const;

  /** Returns the latest result for a given image and ROI and the current settings of the generator.
   @param imageNode
   @param roiNode
   @param onlyIfUpToDate Indicates if results should only be returned if the are up to date, thus not older then image and ROI.
   @param noWIP If noWIP is true, the function only returns valid final result and not just its placeholder (WIP).
   If noWIP equals false it might also return a WIP, thus the valid result is currently processed/ordered but might not be ready yet.*/
  mitk::DataNode::Pointer GetLatestResult(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode, bool onlyIfUpToDate = false, bool noWIP = true) const;

  std::string GenerateStatisticsNodeName(const mitk::Image* image, const mitk::BaseData* roi) const;

  /*! /brief Set flag to ignore zero valued voxels */
  void SetIgnoreZeroValueVoxel(bool _arg);
  /*! /brief Get status of zero value voxel ignoring. */
  bool GetIgnoreZeroValueVoxel() const;

  /*! /brief Set bin size for histogram resolution.*/
  void SetHistogramNBins(unsigned int nbins);
  /*! /brief Get bin size for histogram resolution.*/
  unsigned int GetHistogramNBins() const;

protected:
  bool ChangedNodeIsRelevant(const mitk::DataNode* changedNode) const;
  void IndicateFutureResults(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const;
  std::pair<QmitkDataGenerationJobBase*, mitk::DataNode::Pointer> GetNextMissingGenerationJob(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const;
  void RemoveObsoleteDataNodes(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const;
  mitk::DataNode::Pointer PrepareResultForStorage(const std::string& label, mitk::BaseData* result, const QmitkDataGenerationJobBase* job) const;

  QmitkImageStatisticsDataGenerator(const QmitkImageStatisticsDataGenerator&) = delete;
  QmitkImageStatisticsDataGenerator& operator = (const QmitkImageStatisticsDataGenerator&) = delete;

  bool m_IgnoreZeroValueVoxel = false;
  unsigned int m_HistogramNBins = 100;
};

#endif
