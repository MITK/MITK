/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkImageStatisticsDataGenerator_h
#define QmitkImageStatisticsDataGenerator_h

#include <QmitkImageAndRoiDataGeneratorBase.h>

#include <MitkImageStatisticsUIExports.h>

/**
 * \brief Generates and manages ImageStatisticsContainer results for image/ROI pairs.
 *
 * Uses QmitkImageStatisticsCalculationRunnable jobs to compute statistics for each image/ROI
 * combination and ensures their validity. The generator encodes HistogramNBins and
 * IgnoreZeroValueVoxel as properties on the result containers, since these settings are
 * critical criteria for distinguishing statistics results.
 *
 * For details on the generation orchestration, see QmitkDataGeneratorBase.
 *
 * \sa QmitkImageAndRoiDataGeneratorBase
 * \sa QmitkDataGeneratorBase
 * \sa QmitkImageStatisticsCalculationRunnable
 * \sa mitk::ImageStatisticsContainer
 */
class MITKIMAGESTATISTICSUI_EXPORT QmitkImageStatisticsDataGenerator : public QmitkImageAndRoiDataGeneratorBase
{
public:
  /**
   * \brief Constructs the generator with a data storage.
   * \param[in] storage The data storage for storing results and observing changes.
   * \param[in] parent Optional parent QObject for Qt ownership.
   */
  QmitkImageStatisticsDataGenerator(mitk::DataStorage::Pointer storage, QObject* parent = nullptr) : QmitkImageAndRoiDataGeneratorBase(storage, parent) {};

  /**
   * \brief Constructs the generator without an initial data storage.
   * \param[in] parent Optional parent QObject for Qt ownership.
   */
  QmitkImageStatisticsDataGenerator(QObject* parent = nullptr) : QmitkImageAndRoiDataGeneratorBase(parent) {};

  /**
   * \brief Checks whether a valid, up-to-date statistics result exists for the given image/ROI pair.
   * \param[in] imageNode The image data node.
   * \param[in] roiNode The ROI data node (may be nullptr).
   * \return True if a valid final result is available; false otherwise.
   */
  bool IsValidResultAvailable(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const;

  /**
   * \brief Returns the latest statistics result node for a given image/ROI pair and current settings.
   *
   * \param[in] imageNode The image data node. Must not be nullptr and must contain valid data.
   * \param[in] roiNode The ROI data node (may be nullptr).
   * \param[in] onlyIfUpToDate If true, only returns results that are newer than the source image and ROI.
   * \param[in] noWIP If true, only returns finalized results (not WIP placeholders).
   *                  If false, may also return a WIP placeholder indicating computation is in progress.
   * \return The result data node, or nullptr if no matching result is found.
   * \throw mitk::Exception if imageNode is nullptr or contains no data.
   */
  mitk::DataNode::Pointer GetLatestResult(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode, bool onlyIfUpToDate = false, bool noWIP = true) const;

  /**
   * \brief Generates a unique node name for a statistics result based on image, ROI, and current settings.
   * \param[in] image The source image. Must not be nullptr.
   * \param[in] roi The source ROI data (may be nullptr).
   * \return A string containing the generated node name incorporating bin count, zero-ignore flag, and UIDs.
   * \throw mitk::Exception if image is nullptr.
   */
  std::string GenerateStatisticsNodeName(const mitk::Image* image, const mitk::BaseData* roi) const;

  /**
   * \brief Sets whether zero-valued voxels should be ignored in statistics computation.
   *
   * If auto-update is enabled and the value changes, regeneration is triggered.
   *
   * \param[in] _arg True to ignore zero-valued voxels; false to include them.
   */
  void SetIgnoreZeroValueVoxel(bool _arg);

  /**
   * \brief Returns whether zero-valued voxels are currently being ignored.
   * \return True if zero-valued voxels are ignored; false otherwise.
   */
  bool GetIgnoreZeroValueVoxel() const;

  /**
   * \brief Sets the number of bins for histogram computation.
   *
   * If auto-update is enabled and the value changes, regeneration is triggered.
   *
   * \param[in] nbins The number of histogram bins.
   */
  void SetHistogramNBins(unsigned int nbins);

  /**
   * \brief Returns the current number of histogram bins.
   * \return The number of histogram bins.
   */
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
